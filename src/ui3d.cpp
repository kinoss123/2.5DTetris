#include "ui3d.h"

#include "piece_colors.h"

#include <raylib.h>
#include <rlgl.h>

#include <cmath>

namespace
{
struct LightShader {
    Shader shader{};
    bool loaded = false;
    int locLightDir = -1;
    int locLightColor = -1;
    int locAmbient = -1;
};

static LightShader gLight;

static void LoadLightShaderOnce()
{
    if (gLight.loaded)
        return;

    // Minimal directional light shader. This intentionally stays self-contained
    // (no external asset files).
    const char *vs =
        "#version 330\n"
        "in vec3 vertexPosition;\n"
        "in vec3 vertexNormal;\n"
        "in vec4 vertexColor;\n"
        "uniform mat4 mvp;\n"
        "uniform mat4 matNormal;\n"
        "out vec3 fragNormal;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "  fragNormal = normalize((matNormal*vec4(vertexNormal, 0.0)).xyz);\n"
        "  fragColor = vertexColor;\n"
        "  gl_Position = mvp*vec4(vertexPosition, 1.0);\n"
        "}\n";

    const char *fs = "#version 330\n"
                     "in vec3 fragNormal;\n"
                     "in vec4 fragColor;\n"
                     "uniform vec3 lightDir;\n"
                     "uniform vec3 lightColor;\n"
                     "uniform float ambient;\n"
                     "out vec4 finalColor;\n"
                     "void main() {\n"
                     "  vec3 n = normalize(fragNormal);\n"
                     "  float d = max(dot(n, normalize(-lightDir)), 0.0);\n"
                     "  vec3 lit = fragColor.rgb*(ambient + d)*lightColor;\n"
                     "  finalColor = vec4(lit, fragColor.a);\n"
                     "}\n";

    gLight.shader = LoadShaderFromMemory(vs, fs);
    gLight.locLightDir = GetShaderLocation(gLight.shader, "lightDir");
    gLight.locLightColor = GetShaderLocation(gLight.shader, "lightColor");
    gLight.locAmbient = GetShaderLocation(gLight.shader, "ambient");
    gLight.loaded = (gLight.shader.id != 0);

    if (gLight.loaded) {
        const float lightDir[3] = {-0.4f, -1.0f, -0.2f};
        const float lightColor[3] = {1.0f, 1.0f, 1.0f};
        const float ambient = 0.28f;
        SetShaderValue(gLight.shader, gLight.locLightDir, lightDir,
                       SHADER_UNIFORM_VEC3);
        SetShaderValue(gLight.shader, gLight.locLightColor, lightColor,
                       SHADER_UNIFORM_VEC3);
        SetShaderValue(gLight.shader, gLight.locAmbient, &ambient,
                       SHADER_UNIFORM_FLOAT);
    }
}

static Camera3D MakeFlatCamera(float halfH, float cell)
{
    Camera3D cam{};
    cam.position = {0.0f, 0.0f, 2.45f};
    cam.target = {0.0f, 0.0f, 0.0f};
    cam.up = {0.0f, 1.0f, 0.0f};
    // In orthographic mode, fovy is the vertical size of the viewing volume.
    cam.fovy = (halfH * 2.0f) + (cell * 6.0f);
    cam.projection = CAMERA_ORTHOGRAPHIC;
    return cam;
}

struct Board3DGeom {
    float cell;
    float cube;
    float gap;
    float faceOut;
    float bodyW;
    float bodyH;
    float bodyD;
    float halfW;
    float halfH;
    float halfD;
};

static Board3DGeom MakeBoard3DGeom()
{
    Board3DGeom g{};
    g.cell = 0.08f;
    g.cube = g.cell * 0.92f;
    g.gap = g.cell * 0.05f;
    g.faceOut = (g.cube * 0.5f) + g.gap;

    // The logical grid spans (BOARD_* - 1)*cell between the first and last
    // column/row centers. Add cube size so the outermost cubes fit exactly.
    g.bodyW = ((BOARD_WIDTH - 1) * g.cell) + g.cube;
    g.bodyH = ((BOARD_HEIGHT - 1) * g.cell) + g.cube;
    g.bodyD = g.bodyW;

    g.halfW = g.bodyW * 0.5f;
    g.halfH = g.bodyH * 0.5f;
    g.halfD = g.bodyD * 0.5f;
    return g;
}

static float EaseInOut(float t)
{
    if (t <= 0.0f)
        return 0.0f;
    if (t >= 1.0f)
        return 1.0f;
    // smoothstep
    return t * t * (3.0f - 2.0f * t);
}

static Color RayColorForKind(int kind)
{
    RGBA8 c = PieceRGBAForKind(kind);
    return {c.r, c.g, c.b, c.a};
}
} // namespace

void UI3D::DrawRotate(const Board &board, int fromFace, float angleDeg)
{
    // Simple cube-turn visualization: draw filled blocks on the surfaces.
    // Important: this board is a ring where adjacent faces share edge columns.
    // Map that by placing x=0 and x=BOARD_WIDTH-1 on the cube's vertical edges.
    const Board3DGeom g = MakeBoard3DGeom();

    Camera3D cam = MakeFlatCamera(g.halfH, g.cell);

    BeginMode3D(cam);

    LoadLightShaderOnce();
    if (gLight.loaded)
        BeginShaderMode(gLight.shader);

    // Global cube yaw. Sign is chosen so a CW face step (0 -> 1) makes the
    // right face swing into view (motion reads right-to-left on screen).
    const float globalYawDeg = -((float)fromFace * 90.0f + angleDeg);

    // Determine which faces are visible to the camera (camera is fixed on +Z
    // looking toward origin). In world space, a face is visible if its normal
    // has a +Z component.
    bool faceVisible[4] = {false, false, false, false};
    for (int f = 0; f < 4; f++) {
        float a = ((float)f * 90.0f + globalYawDeg) * DEG2RAD;
        float nz = std::cos(a);
        faceVisible[f] = (nz > 0.0001f);
    }

    // Rotate the entire scene (cube + blocks) so it reads like a rigid cube
    // turning, not stickers sliding around.
    rlPushMatrix();
    rlRotatef(globalYawDeg, 0.0f, 1.0f, 0.0f);

    // For each face, draw filled cells as small cubes glued to that face.
    for (int face = 0; face < 4; face++) {
        if (!faceVisible[face])
            continue;

        // Face normal orientation around Y.
        float faceAngleDeg = (float)face * 90.0f;
        float faceAngle = faceAngleDeg * DEG2RAD;

        for (int y = 0; y < BOARD_HEIGHT; y++) {
            // Interior columns only: exclude edge columns so we can draw the
            // shared-edge columns exactly once as corner columns.
            for (int x = 1; x <= BOARD_WIDTH - 2; x++) {
                if (board.IsFreeBlock(face, x, y))
                    continue;

                Color c = RayColorForKind(board.BlockKind(face, x, y));

                float px = (-g.halfW + (g.cube * 0.5f) + x * g.cell);
                float py = (g.halfH - (g.cube * 0.5f) - y * g.cell);
                // Place cubes inside the cube volume so they never protrude
                // past the outline during rotation.
                float pz = g.halfD - g.faceOut;

                // Rotate local face coords around Y.
                float rx = px * std::cos(faceAngle) + pz * std::sin(faceAngle);
                float rz = -px * std::sin(faceAngle) + pz * std::cos(faceAngle);

                DrawCube({rx, py, rz}, g.cube, g.cube, g.cube, c);
                DrawCubeWires({rx, py, rz}, g.cube, g.cube, g.cube,
                              {40, 40, 46, 255});
            }
        }
    }

    // Shared edge columns (one per cube corner). Each face's x=BOARD_WIDTH-1
    // is the shared column with the next face's x=0, so draw those four
    // columns once.
    for (int face = 0; face < 4; face++) {
        // The shared edge between face and (face+1) is visible if either face
        // is visible.
        if (!faceVisible[face] && !faceVisible[(face + 1) % 4])
            continue;

        float faceAngleDeg = (float)face * 90.0f;
        float faceAngle = faceAngleDeg * DEG2RAD;

        for (int y = 0; y < BOARD_HEIGHT; y++) {
            if (board.IsFreeBlock(face, BOARD_WIDTH - 1, y))
                continue;

            Color c =
                RayColorForKind(board.BlockKind(face, BOARD_WIDTH - 1, y));

            // Corner line for this face's right edge. Keep it inside the cube
            // volume (no protrusion) so the landing frame is smooth.
            float px = g.halfW - g.faceOut;
            float py = (g.halfH - (g.cube * 0.5f) - y * g.cell);
            float pz = g.halfD - g.faceOut;

            float rx = px * std::cos(faceAngle) + pz * std::sin(faceAngle);
            float rz = -px * std::sin(faceAngle) + pz * std::cos(faceAngle);

            DrawCube({rx, py, rz}, g.cube, g.cube, g.cube, c);
            DrawCubeWires({rx, py, rz}, g.cube, g.cube, g.cube,
                          {40, 40, 46, 255});
        }
    }

    rlPopMatrix();

    // End lighting before drawing crisp white overlays.
    if (gLight.loaded)
        EndShaderMode();

    // Overlays (unlit): cube outline + grid.
    rlPushMatrix();
    rlRotatef(globalYawDeg, 0.0f, 1.0f, 0.0f);

    DrawCubeWires({0.0f, 0.0f, 0.0f}, g.bodyW, g.bodyH, g.bodyD, BLUE);

    // Grid for visible faces.
    const Color grid = {255, 255, 255, 90};
    const float eps = 0.0015f;
    const float zFace = g.halfD - g.faceOut;
    const float zGrid = zFace + (g.cube * 0.5f) + eps;

    auto XBound = [&](int i) -> float {
        if (i <= 0)
            return -g.halfW;
        if (i >= BOARD_WIDTH)
            return g.halfW;
        return (-g.halfW + (g.cube * 0.5f) + ((float)i - 0.5f) * g.cell);
    };
    auto YBound = [&](int j) -> float {
        if (j <= 0)
            return g.halfH;
        if (j >= BOARD_HEIGHT)
            return -g.halfH;
        return (g.halfH - (g.cube * 0.5f) - ((float)j - 0.5f) * g.cell);
    };

    for (int face = 0; face < 4; face++) {
        if (!faceVisible[face])
            continue;

        float faceAngleDeg = (float)face * 90.0f;
        float a = faceAngleDeg * DEG2RAD;

        auto ToWorld = [&](float px, float py, float pz) -> Vector3 {
            float rx = px * std::cos(a) + pz * std::sin(a);
            float rz = -px * std::sin(a) + pz * std::cos(a);
            return {rx, py, rz};
        };

        // Vertical lines.
        for (int i = 0; i <= BOARD_WIDTH; i++) {
            float x = XBound(i);
            Vector3 a0 = ToWorld(x, g.halfH, zGrid);
            Vector3 a1 = ToWorld(x, -g.halfH, zGrid);
            DrawLine3D(a0, a1, grid);
        }
        // Horizontal lines.
        for (int j = 0; j <= BOARD_HEIGHT; j++) {
            float y = YBound(j);
            Vector3 a0 = ToWorld(-g.halfW, y, zGrid);
            Vector3 a1 = ToWorld(g.halfW, y, zGrid);
            DrawLine3D(a0, a1, grid);
        }
    }

    rlPopMatrix();

    EndMode3D();
}

void UI3D::DrawActiveFace(const Board &board, const Pieces &pieces,
                          const Game &game)
{
    const int face = board.ActiveFace();
    const Board3DGeom g = MakeBoard3DGeom();
    Camera3D cam = MakeFlatCamera(g.halfH, g.cell);

    BeginMode3D(cam);

    LoadLightShaderOnce();
    if (gLight.loaded)
        BeginShaderMode(gLight.shader);

    auto WorldX = [&](int x) {
        return (-g.halfW + (g.cube * 0.5f) + (float)x * g.cell);
    };
    auto WorldY = [&](int y) {
        return (g.halfH - (g.cube * 0.5f) - (float)y * g.cell);
    };

    const float zFace = g.halfD - g.faceOut;

    // Stored blocks.
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board.IsFreeBlock(face, x, y))
                continue;

            Color c = RayColorForKind(board.BlockKind(face, x, y));
            float px = WorldX(x);
            float py = WorldY(y);
            DrawCube({px, py, zFace}, g.cube, g.cube, g.cube, c);
            DrawCubeWires({px, py, zFace}, g.cube, g.cube, g.cube,
                          {40, 40, 46, 255});
        }
    }

    // Falling piece.
    if (!game.HasActivePiece()) {
        if (gLight.loaded)
            EndShaderMode();

        // Overlays (unlit): cube outline + grid.
        DrawCubeWires({0.0f, 0.0f, 0.0f}, g.bodyW, g.bodyH, g.bodyD, BLUE);

        {
            const Color grid = {255, 255, 255, 90};
            const float eps = 0.0015f;
            float zGrid = zFace + (g.cube * 0.5f) + eps;

            auto XBound = [&](int i) -> float {
                if (i <= 0)
                    return -g.halfW;
                if (i >= BOARD_WIDTH)
                    return g.halfW;
                // Between column i-1 and i.
                return (-g.halfW + (g.cube * 0.5f) +
                        ((float)i - 0.5f) * g.cell);
            };
            auto YBound = [&](int j) -> float {
                if (j <= 0)
                    return g.halfH;
                if (j >= BOARD_HEIGHT)
                    return -g.halfH;
                // Between row j-1 and j.
                return (g.halfH - (g.cube * 0.5f) - ((float)j - 0.5f) * g.cell);
            };

            // Vertical grid lines.
            for (int i = 0; i <= BOARD_WIDTH; i++) {
                float x = XBound(i);
                DrawLine3D({x, g.halfH, zGrid}, {x, -g.halfH, zGrid}, grid);
            }
            // Horizontal grid lines.
            for (int j = 0; j <= BOARD_HEIGHT; j++) {
                float y = YBound(j);
                DrawLine3D({-g.halfW, y, zGrid}, {g.halfW, y, zGrid}, grid);
            }
        }

        EndMode3D();
        return;
    }

    const int piece = game.ActivePiece();
    const int rot = game.ActiveRotation();
    const int x0 = game.ActiveX();
    const int y0 = game.ActiveY();
    Color pieceC = RayColorForKind(piece);

    for (int i = 0; i < PIECE_BLOCKS; i++) {
        for (int j = 0; j < PIECE_BLOCKS; j++) {
            int t = pieces.GetBlockType(piece, rot, j, i);
            if (t == 0)
                continue;
            int bx = x0 + i;
            int by = y0 + j;
            if (by < 0)
                continue;
            if (bx < 0 || bx >= BOARD_WIDTH || by >= BOARD_HEIGHT)
                continue;
            float px = WorldX(bx);
            float py = WorldY(by);
            float zPiece = zFace + (g.cube * 0.12f);
            DrawCube({px, py, zPiece}, g.cube, g.cube, g.cube, pieceC);
            DrawCubeWires({px, py, zPiece}, g.cube, g.cube, g.cube,
                          {40, 40, 46, 255});
        }
    }

    if (gLight.loaded)
        EndShaderMode();

    // Overlays (unlit): cube outline + grid.
    DrawCubeWires({0.0f, 0.0f, 0.0f}, g.bodyW, g.bodyH, g.bodyD, BLUE);

    {
        const Color grid = {255, 255, 255, 90};
        const float eps = 0.0015f;
        float zGrid = zFace + (g.cube * 0.5f) + eps;

        auto XBound = [&](int i) -> float {
            if (i <= 0)
                return -g.halfW;
            if (i >= BOARD_WIDTH)
                return g.halfW;
            // Between column i-1 and i.
            return (-g.halfW + (g.cube * 0.5f) + ((float)i - 0.5f) * g.cell);
        };
        auto YBound = [&](int j) -> float {
            if (j <= 0)
                return g.halfH;
            if (j >= BOARD_HEIGHT)
                return -g.halfH;
            // Between row j-1 and j.
            return (g.halfH - (g.cube * 0.5f) - ((float)j - 0.5f) * g.cell);
        };

        // Vertical grid lines.
        for (int i = 0; i <= BOARD_WIDTH; i++) {
            float x = XBound(i);
            DrawLine3D({x, g.halfH, zGrid}, {x, -g.halfH, zGrid}, grid);
        }
        // Horizontal grid lines.
        for (int j = 0; j <= BOARD_HEIGHT; j++) {
            float y = YBound(j);
            DrawLine3D({-g.halfW, y, zGrid}, {g.halfW, y, zGrid}, grid);
        }
    }

    EndMode3D();
}

void UI3D::Shutdown()
{
    if (!gLight.loaded)
        return;
    UnloadShader(gLight.shader);
    gLight.loaded = false;
}
