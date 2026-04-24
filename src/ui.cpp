#include "ui.h"

#include "piece_colors.h"

#include <cstdio>

void UI::DrawPiece(IO &io, const Board &board, const Pieces &pieces, int x,
                   int y, int piece, int rotation)
{
    IOColor c = PieceIOColorForKind(piece);

    // Small inset so blocks don't visually merge.
    static const int kPad = 1;

    int pixelsX = board.GetXPosInPixels(x);
    int pixelsY = board.GetYPosInPixels(y);

    for (int i = 0; i < PIECE_BLOCKS; i++) {
        for (int j = 0; j < PIECE_BLOCKS; j++) {
            if (pieces.GetBlockType(piece, rotation, j, i) != 0)
                io.DrawRectangle(
                    pixelsX + i * BLOCK_SIZE + kPad,
                    pixelsY + j * BLOCK_SIZE + kPad,
                    (pixelsX + i * BLOCK_SIZE) + BLOCK_SIZE - 1 - kPad,
                    (pixelsY + j * BLOCK_SIZE) + BLOCK_SIZE - 1 - kPad, c);
        }
    }
}

void UI::DrawBoard(IO &io, const Board &board, int screenHeight)
{
    int x1 = BOARD_POSITION - (BLOCK_SIZE * (BOARD_WIDTH / 2)) - 1;
    int x2 = BOARD_POSITION + (BLOCK_SIZE * (BOARD_WIDTH / 2));
    int y = screenHeight - (BLOCK_SIZE * BOARD_HEIGHT);

    io.DrawRectangle(x1 - BOARD_LINE_WIDTH, y, x1, screenHeight - 1,
                     IOColor::Blue);
    io.DrawRectangle(x2, y, x2 + BOARD_LINE_WIDTH, screenHeight - 1,
                     IOColor::Blue);

    x1 += 1;
    static const int kPad = 1;
    for (int i = 0; i < BOARD_WIDTH; i++) {
        for (int j = 0; j < BOARD_HEIGHT; j++) {
            int kind = board.BlockKind(i, j);
            if (kind >= 0)
                io.DrawRectangle(x1 + i * BLOCK_SIZE + kPad,
                                 y + j * BLOCK_SIZE + kPad,
                                 (x1 + i * BLOCK_SIZE) + BLOCK_SIZE - 1 - kPad,
                                 (y + j * BLOCK_SIZE) + BLOCK_SIZE - 1 - kPad,
                                 PieceIOColorForKind(kind));
        }
    }
}

void UI::Draw(IO &io, const Board &board, const Pieces &pieces,
              const Game &game, int screenHeight)
{
    DrawBoard(io, board, screenHeight);
    DrawPiece(io, board, pieces, game.ActiveX(), game.ActiveY(),
              game.ActivePiece(), game.ActiveRotation());

    DrawHud(io, board, pieces, game);
}

void UI::DrawHud(IO &io, const Board &board, const Pieces &pieces,
                 const Game &game)
{
    const int screenW = io.GetScreenWidth();

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

    // Background boxes.
    io.DrawRectangle(x1, scoreY1, x2, scoreY2, IOColor::DarkGray);
    io.DrawRectangle(x1, nextY1, x2, nextY2, IOColor::DarkGray);
    io.DrawRectangle(x1, faceY1, x2, faceY2, IOColor::DarkGray);

    // Borders.
    io.DrawRectangle(x1, scoreY1, x2, scoreY1 + 1, IOColor::LightGray);
    io.DrawRectangle(x1, scoreY2 - 1, x2, scoreY2, IOColor::LightGray);
    io.DrawRectangle(x1, scoreY1, x1 + 1, scoreY2, IOColor::LightGray);
    io.DrawRectangle(x2 - 1, scoreY1, x2, scoreY2, IOColor::LightGray);

    io.DrawRectangle(x1, nextY1, x2, nextY1 + 1, IOColor::LightGray);
    io.DrawRectangle(x1, nextY2 - 1, x2, nextY2, IOColor::LightGray);
    io.DrawRectangle(x1, nextY1, x1 + 1, nextY2, IOColor::LightGray);
    io.DrawRectangle(x2 - 1, nextY1, x2, nextY2, IOColor::LightGray);

    io.DrawRectangle(x1, faceY1, x2, faceY1 + 1, IOColor::LightGray);
    io.DrawRectangle(x1, faceY2 - 1, x2, faceY2, IOColor::LightGray);
    io.DrawRectangle(x1, faceY1, x1 + 1, faceY2, IOColor::LightGray);
    io.DrawRectangle(x2 - 1, faceY1, x2, faceY2, IOColor::LightGray);

    // Score label and value (centered).
    const char *scoreLabel = "SCORE";
    const int scoreLabelSize = 12;
    const int scoreValueSize = 20;
    int scoreLabelW = io.MeasureTextWidth(scoreLabel, scoreLabelSize);
    int scoreLabelX = x1 + (kBoxW - scoreLabelW) / 2;
    io.DrawText(scoreLabelX, scoreY1 + 8, scoreLabel, scoreLabelSize,
                IOColor::White);

    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d", game.Score());
    int scoreValueW = io.MeasureTextWidth(buf, scoreValueSize);
    int scoreValueX = x1 + (kBoxW - scoreValueW) / 2;
    io.DrawText(scoreValueX, scoreY1 + 26, buf, scoreValueSize,
                IOColor::Yellow);

    // Next label (centered).
    const char *nextLabel = "NEXT";
    const int nextLabelSize = 12;
    int nextLabelW = io.MeasureTextWidth(nextLabel, nextLabelSize);
    int nextLabelX = x1 + (kBoxW - nextLabelW) / 2;
    io.DrawText(nextLabelX, nextY1 + 8, nextLabel, nextLabelSize,
                IOColor::White);

    // Next piece preview inside the NEXT box (12px blocks, centered).
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

    // Draw 5x5 block matrix.
    for (int i = 0; i < PIECE_BLOCKS; i++) {
        for (int j = 0; j < PIECE_BLOCKS; j++) {
            int t = pieces.GetBlockType(game.NextPiece(), game.NextRotation(),
                                        j, i);
            if (t == 0)
                continue;

            IOColor c = PieceIOColorForKind(game.NextPiece());
            const int xA = previewX0 + i * kPreviewBlock;
            const int yA = previewY0 + j * kPreviewBlock;
            io.DrawRectangle(xA + 1, yA + 1, xA + kPreviewBlock - 2,
                              yA + kPreviewBlock - 2, c);
        }
    }

    // Face indicator.
    const char *faceLabel = "FACE";
    const int faceLabelSize = 12;
    int faceLabelW = io.MeasureTextWidth(faceLabel, faceLabelSize);
    int faceLabelX = x1 + (kBoxW - faceLabelW) / 2;
    io.DrawText(faceLabelX, faceY1 + 8, faceLabel, faceLabelSize,
                IOColor::White);

    std::snprintf(buf, sizeof(buf), "%d/4", board.ActiveFace() + 1);
    const int faceValueSize = 18;
    int faceValueW = io.MeasureTextWidth(buf, faceValueSize);
    int faceValueX = x1 + (kBoxW - faceValueW) / 2;
    io.DrawText(faceValueX, faceY1 + 26, buf, faceValueSize, IOColor::White);
}
