#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "header_files/stb_image_write.h"
// https://github.com/nothings/stb/blob/master/stb_image_write.h 

#define STB_IMAGE_IMPLEMENTATION
#include "header_files/stb_image.h"
// https://github.com/nothings/stb/blob/master/stb_image.h

#define STB_TRUETYPE_IMPLEMENTATION
#include "header_files/stb_truetype.h"
// https://github.com/nothings/stb/blob/master/stb_truetype.h 

#include <vector>
#include <cstdint>
#include <iostream>
#include <string>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <fstream>

enum class Piece{
    Empty,
    WPawn, WKnight, WBishop, WRook, WQueen, WKing,
    BPawn, BKnight, BBishop, BRook, BQueen, BKing
};

Piece charToPiece(char c){
    switch(c){
        case 'P': return Piece::WPawn;
        case 'N': return Piece::WKnight;
        case 'B': return Piece::WBishop;
        case 'R': return Piece::WRook;
        case 'Q': return Piece::WQueen;
        case 'K': return Piece::WKing;

        case 'p': return Piece::BPawn;
        case 'n': return Piece::BKnight;
        case 'b': return Piece::BBishop;
        case 'r': return Piece::BRook;
        case 'q': return Piece::BQueen;
        case 'k': return Piece::BKing;

        default: return Piece::Empty;
    }
}

const char* pieceToUnicode(Piece p){
    switch (p) {
        case Piece::WKing:   return u8"♔";
        case Piece::WQueen:  return u8"♕";
        case Piece::WRook:   return u8"♖";
        case Piece::WBishop: return u8"♗";
        case Piece::WKnight: return u8"♘";
        case Piece::WPawn:   return u8"♙";

        case Piece::BKing:   return u8"♚";
        case Piece::BQueen:  return u8"♛";
        case Piece::BRook:   return u8"♜";
        case Piece::BBishop: return u8"♝";
        case Piece::BKnight: return u8"♞";
        case Piece::BPawn:   return u8"♟";

        default: return ".";
    }
}

char pieceToChar(Piece p){
    switch (p) {
        case Piece::WKing: return 'K';
        case Piece::WQueen: return 'Q';
        case Piece::WRook: return 'R';
        case Piece::WBishop: return 'B';
        case Piece::WKnight: return 'N';
        case Piece::WPawn: return 'P';

        case Piece::BKing: return 'k';
        case Piece::BQueen: return 'q';
        case Piece::BRook: return 'r';
        case Piece::BBishop: return 'b';
        case Piece::BKnight: return 'n';
        case Piece::BPawn: return 'p';

        default: return '.';
    }
}

void printBoard(Piece board[8][8]){
    for (int rank = 7; rank >= 0; --rank) {
        // std::cout << rank + 1 << " ";

        for (int file = 0; file < 8; ++file) {
            Piece p = board[rank][file];

            if (p == Piece::Empty)
                std::cout << ". ";
            else
                std::cout << pieceToUnicode(board[rank][file]) << " ";
                // std::cout << pieceToChar(board[rank][file]) << " ";
        }

        std::cout << '\n';
    }

    // std::cout << "  a b c d e f g h\n";
}

// void parseFEN(const std::string& fen, Piece board[8][8]){
//     int rank =7;
//     int file =0;
//     for (char c : fen) {
//         if (c == ' ')
//             break;
//
//         if (c == '/') {
//             rank--;
//             file = 0;
//         }
//         else if (isdigit(c)) {
//             int emptySquares = c - '0';
//             for (int i = 0; i < emptySquares; ++i) {
//                 board[rank][file++] = Piece::Empty;
//             }
//         }
//         else {
//             board[rank][file++] = charToPiece(c);
//         }
//     }
// }

struct CastlingRights {
    bool whiteKingSide = false;
    bool whiteQueenSide = false;
    bool blackKingSide = false;
    bool blackQueenSide = false;
};

struct Position {
    Piece board[8][8]{};
    bool whiteToMove = true;
    CastlingRights castling;
    int enPassantFile = -1; // 0=a, 1=b, ..., 7=h
    int enPassantRank = -1; // 0=rank 1, ..., 7=rank 8
    int halfmoveClock = 0;
    int fullmoveNumber = 1;
};

void parseBoardField(const std::string& boardField, Piece board[8][8]){
    for (int r = 0; r < 8; ++r)
        for (int f = 0; f < 8; ++f)
            board[r][f] = Piece::Empty;

    int rank = 7;
    int file = 0;

    for (char c : boardField) {
        if (c == '/') {
            if (file != 8)
                throw std::runtime_error("Invalid FEN: rank does not contain 8 files");

            rank--;
            file = 0;

            if (rank < 0)
                throw std::runtime_error("Invalid FEN: too many ranks");
        }
        else if (std::isdigit(static_cast<unsigned char>(c))) {
            int emptyCount = c - '0';

            if (emptyCount < 1 || emptyCount > 8)
                throw std::runtime_error("Invalid FEN: invalid empty-square number");

            if (file + emptyCount > 8)
                throw std::runtime_error("Invalid FEN: too many files in rank");

            for (int i = 0; i < emptyCount; ++i) {
                board[rank][file++] = Piece::Empty;
            }
        }
        else {
            if (file >= 8)
                throw std::runtime_error("Invalid FEN: too many files in rank");

            board[rank][file++] = charToPiece(c);
        }
    }

    if (rank != 0 || file != 8)
        throw std::runtime_error("Invalid FEN: expected exactly 8 ranks");
}

Position parseFEN(const std::string& fen){
    std::istringstream ss(fen);

    std::string boardField;
    std::string sideField;
    std::string castlingField;
    std::string enPassantField;
    std::string halfmoveField;
    std::string fullmoveField;

    if (!(ss >> boardField >> sideField >> castlingField
             >> enPassantField >> halfmoveField >> fullmoveField)) {
        throw std::runtime_error("Invalid FEN: expected 6 fields");
    }

    Position pos;

    parseBoardField(boardField, pos.board);

    if (sideField == "w") {
        pos.whiteToMove = true;
    }
    else if (sideField == "b") {
        pos.whiteToMove = false;
    }
    else {
        throw std::runtime_error("Invalid FEN: side to move must be w or b");
    }

    if (castlingField != "-") {
        for (char c : castlingField) {
            switch (c) {
                case 'K': pos.castling.whiteKingSide = true; break;
                case 'Q': pos.castling.whiteQueenSide = true; break;
                case 'k': pos.castling.blackKingSide = true; break;
                case 'q': pos.castling.blackQueenSide = true; break;
                default:
                    throw std::runtime_error("Invalid FEN: bad castling rights");
            }
        }
    }

    if (enPassantField == "-") {
        pos.enPassantFile = -1;
        pos.enPassantRank = -1;
    }
    else {
        if (enPassantField.size() != 2)
            throw std::runtime_error("Invalid FEN: bad en passant square");

        char fileChar = enPassantField[0];
        char rankChar = enPassantField[1];

        if (fileChar < 'a' || fileChar > 'h')
            throw std::runtime_error("Invalid FEN: bad en passant file");

        if (rankChar < '1' || rankChar > '8')
            throw std::runtime_error("Invalid FEN: bad en passant rank");

        pos.enPassantFile = fileChar - 'a';
        pos.enPassantRank = rankChar - '1';
    }

    pos.halfmoveClock = std::stoi(halfmoveField);
    pos.fullmoveNumber = std::stoi(fullmoveField);

    return pos;
}

std::string pieceToImagePath(Piece p){
    const std::string dir="pieces-basic-png/";
    // all png availabe as "Standard chess pieces only" in link below
    // https://greenchess.net/info.php?item=downloads 
    switch(p){
        case Piece::WPawn:   return dir+"white-pawn.png";
        case Piece::WKnight: return dir+"white-knight.png";
        case Piece::WBishop: return dir+"white-bishop.png";
        case Piece::WRook:   return dir+"white-rook.png";
        case Piece::WQueen:  return dir+"white-queen.png";
        case Piece::WKing:   return dir+"white-king.png";

        case Piece::BPawn:   return dir+"black-pawn.png";
        case Piece::BKnight: return dir+"black-knight.png";
        case Piece::BBishop: return dir+"black-bishop.png";
        case Piece::BRook:   return dir+"black-rook.png";
        case Piece::BQueen:  return dir+"black-queen.png";
        case Piece::BKing:   return dir+"black-king.png";

        default: return "";
    }
}

void drawPieceCentered(std::vector<uint8_t>& boardPixels,int boardWidth,int boardHeight,Piece piece,int squareX,int squareY,int squareSize){
    std::string path = pieceToImagePath(piece);
    if(path.empty()){return;}

    int imgW, imgH, channels;

    // Force loaded image to RGBA: 4 channels
    uint8_t* piecePixels = stbi_load(path.c_str(), &imgW, &imgH, &channels, 4);

    if (!piecePixels) {
        std::cerr << "Failed to load piece image: " << path << '\n';
        return;
    }

    int targetSize = static_cast<int>(squareSize);

    int offsetX = squareX + (squareSize - targetSize) / 2;
    int offsetY = squareY + (squareSize - targetSize) / 2;

    for (int y = 0; y < targetSize; ++y) {
        for (int x = 0; x < targetSize; ++x) {
            int srcX = x * imgW / targetSize;
            int srcY = y * imgH / targetSize;

            int srcIndex = (srcY * imgW + srcX) * 4;

            uint8_t srcR = piecePixels[srcIndex + 0];
            uint8_t srcG = piecePixels[srcIndex + 1];
            uint8_t srcB = piecePixels[srcIndex + 2];
            uint8_t srcA = piecePixels[srcIndex + 3];

            if (srcA == 0) continue;

            int dstX = offsetX + x;
            int dstY = offsetY + y;

            if (dstX < 0 || dstX >= boardWidth || dstY < 0 || dstY >= boardHeight)
                continue;

            int dstIndex = (dstY * boardWidth + dstX) * 3;

            float alpha = srcA / 255.0f;

            boardPixels[dstIndex + 0] =
                static_cast<uint8_t>(srcR * alpha + boardPixels[dstIndex + 0] * (1.0f - alpha));

            boardPixels[dstIndex + 1] =
                static_cast<uint8_t>(srcG * alpha + boardPixels[dstIndex + 1] * (1.0f - alpha));

            boardPixels[dstIndex + 2] =
                static_cast<uint8_t>(srcB * alpha + boardPixels[dstIndex + 2] * (1.0f - alpha));
        }
    }

    stbi_image_free(piecePixels);
}

std::vector<unsigned char> loadFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Could not open font file: " + path);
    }

    return std::vector<unsigned char>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

void drawText(std::vector<uint8_t>& pixels,int width,int height,const std::string& text,int x,int baselineY,uint8_t color[3],stbtt_fontinfo& font,float fontSize){
    float scale = stbtt_ScaleForPixelHeight(&font, fontSize);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);

    int penX = x;

    for (char c : text) {
        int glyphIndex = stbtt_FindGlyphIndex(&font, c);

        int glyphW, glyphH, xOffset, yOffset;
        unsigned char* bitmap = stbtt_GetGlyphBitmap(&font,0,scale,glyphIndex,&glyphW,&glyphH,&xOffset,&yOffset);

        for (int gy = 0; gy < glyphH; ++gy) {
            for (int gx = 0; gx < glyphW; ++gx) {
                int alpha = bitmap[gy * glyphW + gx];

                int dstX = penX + xOffset + gx;
                int dstY = baselineY + yOffset + gy;

                if (dstX < 0 || dstX >= width || dstY < 0 || dstY >= height)
                    continue;

                int idx = (dstY * width + dstX) * 3;
                float a = alpha / 255.0f;

                pixels[idx + 0] = color[0] * a + pixels[idx + 0] * (1.0f - a);
                pixels[idx + 1] = color[1] * a + pixels[idx + 1] * (1.0f - a);
                pixels[idx + 2] = color[2] * a + pixels[idx + 2] * (1.0f - a);
            }
        }

        stbtt_FreeBitmap(bitmap, nullptr);

        int advance, leftBearing;
        stbtt_GetGlyphHMetrics(&font, glyphIndex, &advance, &leftBearing);

        penX += static_cast<int>(advance * scale);
    }
}

bool generateBoardPNG(const char* filename, Piece board[8][8],int squareSize = 200){

    // const int boardSize = 8 * squareSize;
    // std::vector<uint8_t> pixels(boardSize * boardSize * 3);

    const int borderSize = 36;
    const int boardOnlySize = 8 * squareSize;
    const int imageSize = boardOnlySize + 2 * borderSize;
    std::vector<uint8_t> pixels(imageSize * imageSize * 3);

    uint8_t borderColor[3] = {40, 55, 75};      // dark blue-gray
    uint8_t textColor[3]   = {240, 240, 240};   // light text

    uint8_t light[3] = {240, 217, 181}; // lighter brown
    uint8_t dark[3]  = {181, 136, 99}; // darker brown


    // draw image background
    for(int y = 0; y < boardOnlySize; ++y){
        for(int x = 0; x < boardOnlySize; ++x){
            int file = x / squareSize;
            int rankFromTop = y / squareSize;

            bool isLightSquare = (file + rankFromTop) % 2 != 0;
            uint8_t* color = isLightSquare ? light : dark;

            int dstX = x + borderSize;
            int dstY = y + borderSize;

            int index = (dstY * imageSize + dstX) * 3;
            pixels[index + 0] = color[0];
            pixels[index + 1] = color[1];
            pixels[index + 2] = color[2];
        }
    }

    // draw board squares
    for (int y = 0; y < boardOnlySize; ++y) {
        for (int x = 0; x < boardOnlySize; ++x) {
            int file = x / squareSize;
            int rankFromTop = y / squareSize;

            bool isLightSquare = (file + rankFromTop) % 2 != 0;
            uint8_t* color = isLightSquare ? light : dark;

            int dstX = x + borderSize;
            int dstY = y + borderSize;

            int index = (dstY * imageSize + dstX) * 3;
            pixels[index + 0] = color[0];
            pixels[index + 1] = color[1];
            pixels[index + 2] = color[2];
        }
    }

    // Draw pieces here later
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            Piece piece = board[rank][file];

            if(piece == Piece::Empty){continue;}

            // int squareX = file * squareSize;
            // int squareY = (7 - rank) * squareSize;

            int squareX = borderSize + file * squareSize;
            int squareY = borderSize + (7 - rank) * squareSize;

            drawPieceCentered(pixels, imageSize, imageSize, piece, squareX, squareY, squareSize);
        }
    }

    // initialize font
    std::vector<unsigned char> fontBuffer = loadFile("/Users/nicky/Desktop/ChessEngine/arial.ttf");
    stbtt_fontinfo font;

    if(!stbtt_InitFont(&font,fontBuffer.data(),stbtt_GetFontOffsetForIndex(fontBuffer.data(), 0))){throw std::runtime_error("Failed to initialize font");}

    float fontSize = borderSize * 0.75f;

    // files: a-h
    for (int file = 0; file < 8; ++file) {
        std::string label(1, 'a' + file);

        int x = borderSize + file * squareSize + squareSize / 2 - 5;
        int y = borderSize + boardOnlySize + borderSize / 2 + 6;

        drawText(pixels, imageSize, imageSize, label, x, y, textColor, font, fontSize);
    }

    // ranks: 1-8
    for (int rank = 0; rank < 8; ++rank) {
        std::string label(1, '1' + rank);

        int x = borderSize / 2 - 4;
        int y = borderSize + (7 - rank) * squareSize + squareSize / 2 + 6;

        drawText(pixels, imageSize, imageSize, label, x, y, textColor, font, fontSize);
    }

    // saving image of board as png
    int success = stbi_write_png(filename,imageSize,imageSize,3,pixels.data(),imageSize * 3);

    return success != 0;
}

int BoardVisualizer(std::string fen, const char* filename){

    Piece board[8][8];

    Position pos = parseFEN(fen);

    // printBoard(board);

    generateBoardPNG(filename, pos.board);

    return 0;
}