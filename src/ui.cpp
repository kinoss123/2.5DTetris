#include "ui.h"

#include "piece_colors.h"

#include <cstdio>

void UI::DrawPiece(Renderer &r, const Board &board, const Pieces &pieces, int x,
                   int y, int piece, int rotation)
{
    RenderColor c = PieceRenderColorForKind(piece);

    // Small inset so blocks don't visually merge.
    static const int kPad = 1;

    int pixelsX = board.GetXPosInPixels(x);
    int pixelsY = board.GetYPosInPixels(y);

    for (int i = 0; i < PIECE_BLOCKS; i++) {
        for (int j = 0; j < PIECE_BLOCKS; j++) {
            if (pieces.GetBlockType(piece, rotation, j, i) != 0)
                r.DrawRectangle(
                    pixelsX + i * BLOCK_SIZE + kPad,
                    pixelsY + j * BLOCK_SIZE + kPad,
                    (pixelsX + i * BLOCK_SIZE) + BLOCK_SIZE - 1 - kPad,
                    (pixelsY + j * BLOCK_SIZE) + BLOCK_SIZE - 1 - kPad, c);
        }
    }
}

void UI::DrawBoard(Renderer &r, const Board &board, int screenHeight)
{
    int x1 = BOARD_POSITION - (BLOCK_SIZE * (BOARD_WIDTH / 2)) - 1;
    int x2 = BOARD_POSITION + (BLOCK_SIZE * (BOARD_WIDTH / 2));
    int y = screenHeight - (BLOCK_SIZE * BOARD_HEIGHT);

    r.DrawRectangle(x1 - BOARD_LINE_WIDTH, y, x1, screenHeight - 1,
                    RenderColor::Blue);
    r.DrawRectangle(x2, y, x2 + BOARD_LINE_WIDTH, screenHeight - 1,
                    RenderColor::Blue);

    x1 += 1;
    static const int kPad = 1;
    for (int i = 0; i < BOARD_WIDTH; i++) {
        for (int j = 0; j < BOARD_HEIGHT; j++) {
            int kind = board.BlockKind(i, j);
            if (kind >= 0)
                r.DrawRectangle(x1 + i * BLOCK_SIZE + kPad,
                                y + j * BLOCK_SIZE + kPad,
                                (x1 + i * BLOCK_SIZE) + BLOCK_SIZE - 1 - kPad,
                                (y + j * BLOCK_SIZE) + BLOCK_SIZE - 1 - kPad,
                                PieceRenderColorForKind(kind));
        }
    }
}

void UI::Draw(Renderer &r, const Board &board, const Pieces &pieces,
              const Game &game, int screenHeight)
{
    DrawBoard(r, board, screenHeight);
    DrawPiece(r, board, pieces, game.ActiveX(), game.ActiveY(),
              game.ActivePiece(), game.ActiveRotation());

    DrawHud(r, board, pieces, game);
}

void UI::DrawHud(Renderer &r, const Board &board, const Pieces &pieces,
                 const Game &game)
{
    const int screenW = r.GetScreenWidth();

    const int kMargin = 12;
    const int kGap = 10;

    const int kBoxW = 120;
    const int kScoreBoxH = 68;
    const int kNextBoxH = 110;
    const int kFaceBoxH = 58;

    const int x2 = screenW - kMargin;
    const int x1 = x2 - kBoxW;
    const int scoreY1 = kMargin;
    const int scoreY2 = scoreY1 + kScoreBoxH;
    const int nextY1 = scoreY2 + kGap;
    const int nextY2 = nextY1 + kNextBoxH;
    const int faceY1 = nextY2 + kGap;
    const int faceY2 = faceY1 + kFaceBoxH;

    r.DrawRectangle(x1, scoreY1, x2, scoreY2, RenderColor::DarkGray);
    r.DrawRectangle(x1, nextY1, x2, nextY2, RenderColor::DarkGray);
    r.DrawRectangle(x1, faceY1, x2, faceY2, RenderColor::DarkGray);

    // Borders.
    r.DrawRectangle(x1, scoreY1, x2, scoreY1 + 1, RenderColor::LightGray);
    r.DrawRectangle(x1, scoreY2 - 1, x2, scoreY2, RenderColor::LightGray);
    r.DrawRectangle(x1, scoreY1, x1 + 1, scoreY2, RenderColor::LightGray);
    r.DrawRectangle(x2 - 1, scoreY1, x2, scoreY2, RenderColor::LightGray);

    r.DrawRectangle(x1, nextY1, x2, nextY1 + 1, RenderColor::LightGray);
    r.DrawRectangle(x1, nextY2 - 1, x2, nextY2, RenderColor::LightGray);
    r.DrawRectangle(x1, nextY1, x1 + 1, nextY2, RenderColor::LightGray);
    r.DrawRectangle(x2 - 1, nextY1, x2, nextY2, RenderColor::LightGray);

    r.DrawRectangle(x1, faceY1, x2, faceY1 + 1, RenderColor::LightGray);
    r.DrawRectangle(x1, faceY2 - 1, x2, faceY2, RenderColor::LightGray);
    r.DrawRectangle(x1, faceY1, x1 + 1, faceY2, RenderColor::LightGray);
    r.DrawRectangle(x2 - 1, faceY1, x2, faceY2, RenderColor::LightGray);

    const char *scoreLabel = "SCORE";
    const int scoreLabelSize = 16;
    const int scoreValueSize = 32;
    int scoreLabelW = r.MeasureTextWidth(scoreLabel, scoreLabelSize);
    int scoreLabelX = x1 + (kBoxW - scoreLabelW) / 2;
    r.DrawText(scoreLabelX, scoreY1 + 8, scoreLabel, scoreLabelSize,
               RenderColor::White);

    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d", game.Score());
    int scoreValueW = r.MeasureTextWidth(buf, scoreValueSize);
    int scoreValueX = x1 + (kBoxW - scoreValueW) / 2;
    r.DrawText(scoreValueX, scoreY1 + 26, buf, scoreValueSize,
               RenderColor::Yellow);

    const char *nextLabel = "NEXT";
    const int nextLabelSize = 16;
    int nextLabelW = r.MeasureTextWidth(nextLabel, nextLabelSize);
    int nextLabelX = x1 + (kBoxW - nextLabelW) / 2;
    r.DrawText(nextLabelX, nextY1 + 8, nextLabel, nextLabelSize,
               RenderColor::White);

    static const int kPreviewBlock = 12;
    static const int kPreviewPad = 8;
    const int boxInnerX1 = x1 + kPreviewPad;
    const int boxInnerX2 = x2 - kPreviewPad;
    const int boxInnerY1 = nextY1 + 26;
    const int boxInnerY2 = nextY2 - kPreviewPad;

    const int previewW = PIECE_BLOCKS * kPreviewBlock;
    const int previewH = PIECE_BLOCKS * kPreviewBlock;
    const int previewX0 =
        boxInnerX1 + ((boxInnerX2 - boxInnerX1) - previewW) / 2;
    const int previewY0 =
        boxInnerY1 + ((boxInnerY2 - boxInnerY1) - previewH) / 2;

    for (int i = 0; i < PIECE_BLOCKS; i++) {
        for (int j = 0; j < PIECE_BLOCKS; j++) {
            int t = pieces.GetBlockType(game.NextPiece(), game.NextRotation(),
                                        j, i);
            if (t == 0)
                continue;

            RenderColor c = PieceRenderColorForKind(game.NextPiece());
            const int xA = previewX0 + i * kPreviewBlock;
            const int yA = previewY0 + j * kPreviewBlock;
            r.DrawRectangle(xA + 1, yA + 1, xA + kPreviewBlock - 2,
                            yA + kPreviewBlock - 2, c);
        }
    }

    const char *faceLabel = "FACE";
    const int faceLabelSize = 16;
    int faceLabelW = r.MeasureTextWidth(faceLabel, faceLabelSize);
    int faceLabelX = x1 + (kBoxW - faceLabelW) / 2;
    r.DrawText(faceLabelX, faceY1 + 8, faceLabel, faceLabelSize,
               RenderColor::White);

    std::snprintf(buf, sizeof(buf), "%d/4", board.ActiveFace() + 1);
    const int faceValueSize = 24;
    int faceValueW = r.MeasureTextWidth(buf, faceValueSize);
    int faceValueX = x1 + (kBoxW - faceValueW) / 2;
    r.DrawText(faceValueX, faceY1 + 26, buf, faceValueSize,
               RenderColor::White);
}
