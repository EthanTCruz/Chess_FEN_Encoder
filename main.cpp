#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>
#include <bitset>
#include <cmath>

using namespace std;

struct pieceBitMap{
    unordered_map<string, bitset<4>> globalHashMap;
    unordered_map<char, int> lexInt;


    pieceBitMap() :
        globalHashMap({
            {"K", bitset<4>(0)}, 
            {"Q", bitset<4>(1)},
            {"R", bitset<4>(2)},
            {"B", bitset<4>(3)},
            {"N", bitset<4>(4)},
            {"P", bitset<4>(5)},
            {"k", bitset<4>(6)},
            {"q", bitset<4>(7)},
            {"r", bitset<4>(8)},
            {"b", bitset<4>(9)},
            {"n", bitset<4>(10)},
            {"p", bitset<4>(11)}
        }),
        lexInt({
            {'a', 0}, 
            {'b', 1},
            {'c', 2},
            {'d', 3},
            {'e', 4},
            {'f', 5},
            {'g', 6},
            {'h', 7}
            }) {};

};


// The Board struct stores a chess board's information.
struct Board {
    // We represent the board as an 8x8 vector of characters.
    // Each rank is a vector of 8 chars; empty squares will be stored as a space ' '.
    vector<vector<char>> squares;
    
    // Other fields from the FEN string.
    char activeColor;         // 'w' or 'b'
    string castlingRights;    // e.g., "KQkq" or "-"
    string enPassant;         // e.g., "e3" or "-"
    int halfmoveClock;
    int fullmoveNumber;
    
    Board() : squares(8, vector<char>(8, ' ')),
              activeColor('w'),
              castlingRights("-"),
              enPassant("-"),
              halfmoveClock(0),
              fullmoveNumber(1) {}
};

void insertBitset(std::bitset<120>& bigSet, const std::bitset<4>& smallSet, std::size_t position) {
    position = position * 4;  // Convert position to bit index
    if (position + smallSet.size() > bigSet.size()) {
        std::cerr << "Error: Position out of bounds!" << std::endl;
        return;
    }

    // Iterate over the small bitset and copy bits to the big bitset
    for (std::size_t i = 0; i < smallSet.size(); ++i) {
        // cout << "Original FEN: " << smallSet[i] << "\n";
        bigSet.set(position + i, smallSet[i]);  // Set bit at correct index
    }
}

bitset<6> getKingPosition(int position) {
    int row = position / 8;
    int col = position % 8;
    bitset<3> rowb = bitset<3>(row);
    bitset<3> colb = bitset<3>(col);
    bitset<6> full;
    

    // Iterate over the small bitset and copy bits to the big bitset
    full = (rowb.to_ulong() << 3) | colb.to_ulong(); // Combine row and column bitsets

    return full;
}

// Decode a FEN string into a Board.
bitset<120> encodeFEN(const string& fen) {
    Board board;
    istringstream iss(fen);
    
    // Split the FEN string into its components
    vector<string> parts;
    string segment;
    while (iss >> segment) {
        parts.push_back(segment);
    }

    if (parts.size() < 6) {
        cerr << "Error: Invalid FEN string." << endl;
        return bitset<120>();
    }

    // Initialize bitsets
    bitset<64> boardBits;      // Represents which squares contain a piece
    bitset<120> pieceType;     // 4 bits * 30 pieces (excluding kings)
    bitset<12> kingPosition;   // Stores 6-bit positions for white and black king
    bitset<6> whiteKingPosBits, blackKingPosBits;
    pieceBitMap pieceMap;

    // === 1st Loop: Process the Non-Board Components ===
    char activeColor = parts[1][0];  // 'w' or 'b'
    string castlingRights = parts[2]; // Castling availability
    string enPassant = parts[3];      // En passant target square
    int halfmoveClock = stoi(parts[4]);
    int fullmoveNumber = stoi(parts[5]);

    // Print the extracted fields (for debugging)
    cout << "Active Color: " << activeColor << endl;
    cout << "Castling Rights: " << castlingRights << endl;
    cout << "En Passant: " << enPassant << endl;
    cout << "Halfmove Clock: " << halfmoveClock << endl;
    cout << "Fullmove Number: " << fullmoveNumber << endl;

    // === 2nd Loop: Process the Board Pieces ===
    string boardPart = parts[0];
    int boardPos = 0;       // Board index from 0-63
    int piecesFound = 0;    // Number of pieces encountered

    for (char c : boardPart) {
        if (c == '/') {
            continue;  // Skip rank separators
        } else if (isdigit(c)) {
            // Skip empty squares based on number
            int emptyCount = c - '0';
            boardPos += emptyCount;
        } else {
            // If piece is found, encode it
            if (c == 'K') {
                whiteKingPosBits = getKingPosition(boardPos);
                for (size_t j = 0; j < 6; ++j) {
                    kingPosition.set(j, whiteKingPosBits[j]);
                }
            } else if (c == 'k') {
                blackKingPosBits = getKingPosition(boardPos);
                for (size_t j = 0; j < 6; ++j) {
                    kingPosition.set(kingPosition.size() - 6 + j, blackKingPosBits[j]);
                }
            } else {
                insertBitset(pieceType, pieceMap.globalHashMap[string(1, c)], piecesFound);
                piecesFound++;
            }
            boardPos++; // Move to the next board position
        }
    }

return pieceType;
}



// Decode a FEN string into a Board.
Board fenToBoard(const string& fen) {
    Board board;
    istringstream iss(fen);
    string boardPart;
    
    // The first part is the piece placement.
    iss >> boardPart;
    
    // FEN piece placement goes rank by rank (from rank 8 to rank 1) separated by '/'
    // For simplicity, we store rank 0 as the top (8th rank) and rank 7 as the bottom.
    int rank = 0, file = 0;
    for (char c : boardPart) {
        if (c == '/') {
            rank++;    // move to the next rank
            file = 0;
        } else if (isdigit(c)) {
            // c is a digit indicating that many empty squares.
            int emptyCount = c - '0';
            for (int i = 0; i < emptyCount; i++) {
                board.squares[rank][file++] = ' '; // use a space for empty squares
            }
        } else {
            // Otherwise, it is a piece letter.
            board.squares[rank][file++] = c;
        }
    }
    
    // Read the other fields.
    iss >> board.activeColor;
    iss >> board.castlingRights;
    iss >> board.enPassant;
    iss >> board.halfmoveClock;
    iss >> board.fullmoveNumber;
    
    return board;
}

// Encode a Board into a FEN string.
string boardToFen(const Board& board) {
    string fen;
    // Encode the piece placement:
    // Traverse rank 0 (8th rank) to rank 7 (1st rank)
    for (int rank = 0; rank < 8; rank++) {
        int emptyCount = 0;
        for (int file = 0; file < 8; file++) {
            char piece = board.squares[rank][file];
            if (piece == ' ') {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += to_string(emptyCount);
                    emptyCount = 0;
                }
                fen.push_back(piece);
            }
        }
        if (emptyCount > 0) {
            fen += to_string(emptyCount);
        }
        if (rank != 7) {
            fen.push_back('/');
        }
    }
    
    // Append the other fields.
    fen += " ";
    fen.push_back(board.activeColor);
    fen += " " + board.castlingRights;
    fen += " " + board.enPassant;
    fen += " " + to_string(board.halfmoveClock);
    fen += " " + to_string(board.fullmoveNumber);
    
    return fen;
}

// A helper function to print the board (for debugging purposes).
void printBoard(const Board& board) {
    cout << "Board Position:\n";
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            cout << board.squares[rank][file] << " ";
        }
        cout << "\n";
    }
}



void printPieceType(const std::bitset<120>& bitset) {
    for (int i = 0; i < bitset.size(); ++i) {
        cout << bitset[i] << "";
    }
    cout << "\n";
}

int main() {
    // Example FEN string for the starting position.
    string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    // Decode the FEN.
    Board board = fenToBoard(fen);
    
    // Print the board.
    // printBoard(board);
    
    // Re-encode the board back into FEN.
    string encoded = boardToFen(board);
    // cout << "\nEncoded FEN:\n" << encoded << "\n";
    bitset<120> pieceType;
    // pieceBitMap pieceMap;
    // insertBitset(pieceType,pieceMap.globalHashMap["p"],0);
    pieceType = encodeFEN(fen);

    printPieceType(pieceType);

    // cout << "Original FEN: " << pieceType << "\n";
    return 0;
}
