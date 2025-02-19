#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>
#include <bitset>
#include <tuple>
#include <cctype>
using namespace std;

struct FenPieceEncoder {
   unordered_map<string, bitset<4>> pieceMapping;
   unordered_map<char, int> lexMapping;
   FenPieceEncoder() :
       pieceMapping({
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
           {"p", bitset<4>(11)},
           {"p with ep", bitset<4>(12)},
           {"r with cr", bitset<4>(13)},
           {"P with ep", bitset<4>(14)},
           {"R with cr", bitset<4>(15)},
       }),
       lexMapping({
           {'a', 0}, {'b', 1}, {'c', 2}, {'d', 3},
           {'e', 4}, {'f', 5}, {'g', 6}, {'h', 7}
       }) {}
};
struct FenPieceDecoder {
   unordered_map<bitset<4>, string> pieceMapping;
   unordered_map<char, int> lexMapping;
   FenPieceDecoder() :
       pieceMapping({
           {bitset<4>(0), "K"},
           {bitset<4>(1), "Q"},
           {bitset<4>(2), "R"},
           {bitset<4>(3), "B"},
           {bitset<4>(4), "N"},
           {bitset<4>(5), "P"},
           {bitset<4>(6), "k"},
           {bitset<4>(7), "q"},
           {bitset<4>(8), "r"},
           {bitset<4>(9), "b"},
           {bitset<4>(10), "n"},
           {bitset<4>(11), "p"},
           {bitset<4>(12), "p with ep"},
           {bitset<4>(13), "r with cr"},
           {bitset<4>(14), "P with ep"},
           {bitset<4>(15), "R with cr"},
       }),
       lexMapping({
           {'a', 0}, {'b', 1}, {'c', 2}, {'d', 3},
           {'e', 4}, {'f', 5}, {'g', 6}, {'h', 7}
       }) {}
};
struct ChessBoard {
   vector<vector<char>> squares;
   char activeColor;
   string castlingRights;
   string enPassant;
   int halfmoveClock;
   int fullmoveNumber;
   ChessBoard() : squares(8, vector<char>(8, ' ')),
                  activeColor('w'),
                  castlingRights("-"),
                  enPassant("-"),
                  halfmoveClock(0),
                  fullmoveNumber(1) {}
};

void insertSubBitset(bitset<120>& bigSet, const bitset<4>& smallSet, size_t position) {
   position *= 4;  // Convert position to bit index
   if (position + smallSet.size() > bigSet.size()) {
       cerr << "Error: Position out of bounds!" << endl;
       return;
   }
   for (size_t i = 0; i < smallSet.size(); ++i)
       bigSet.set(position + i, smallSet[i]);
}
int fenSquareToIndex(const string& square,const char& turn) {
   if (square == "-")
       return -1;
   int file = square[0] - 'a';
   
   int rank = square[1] - '1' + 1;
   cout <<"rank: " << rank << " file: " << file << " square: " << square <<"\n" ;
//    rank += (turn == 'w' ? -1 : 1);
   return (8-rank)*8 + file;
}
int getEnPassantIndex(const string& enPassant, const char& turn) {
   int index = fenSquareToIndex(enPassant,turn);
   if (index == -1){return index;}
   cout << "original index: " << index << "\n";
   index += (turn == 'w' ? 8 : -8) ;
   cout << "ep index: " << index << "\n";
   return index;
}

tuple<bool, bool, bool, bool> parseCastlingRights(const string& castlingRights) {
   bool whiteKingSide = (castlingRights.find('K') != string::npos);
   bool blackKingSide = (castlingRights.find('k') != string::npos);
   bool whiteQueenSide = (castlingRights.find('Q') != string::npos);
   bool blackQueenSide = (castlingRights.find('q') != string::npos);
   return make_tuple(whiteKingSide, blackKingSide, whiteQueenSide, blackQueenSide);
}

tuple<char, tuple<bool, bool, bool, bool>, string, int, int, int, string> dismantleFEN(const string& fen) {
    istringstream iss(fen);
    vector<string> parts;
    string segment;
    while (iss >> segment)
        parts.push_back(segment);
    if (parts.size() < 6) {
        cerr << "Error: Invalid FEN string." << endl;
        return make_tuple(' ', make_tuple(false, false, false, false), "", 0, 0, -1, "");
    }

    char activeColor = parts[1][0];
    string castlingRights = parts[2];
    string enPassant = parts[3];
    int halfmoveClock = stoi(parts[4]);
    int fullmoveNumber = stoi(parts[5]);
    int epSquare = getEnPassantIndex(enPassant, activeColor);
    auto castlingRightsParsed = parseCastlingRights(castlingRights);
    string boardPart = parts[0];

    return make_tuple(activeColor, castlingRightsParsed, enPassant, halfmoveClock, fullmoveNumber, epSquare, boardPart);
}

tuple<bitset<120>, bitset<62>, bitset<12>> encodeFenToBitboards(const string& fen) {
   istringstream iss(fen);
   vector<string> parts;
   string segment;
   while (iss >> segment)
       parts.push_back(segment);
   if (parts.size() < 6) {
       cerr << "Error: Invalid FEN string." << endl;
       return make_tuple(bitset<120>(), bitset<62>(), bitset<12>());
   }
   bitset<62> occupancyBitset;
   bitset<120> pieceTypeBitset;
   bitset<12> kingPositionBitset;
   bitset<6> whiteKingBitset, blackKingBitset;
   bool wKingfound = false, bKingFound = false;
   FenPieceEncoder fenEncoder;
   auto[activeColor, castlingRightsParsed, enPassant, halfmoveClock, fullmoveNumber, epSquare, boardPart] = dismantleFEN(fen);
   int boardPos = 0, piecesFound = 0, truePos = 0;

   for (char c : boardPart) {
       if (c == '/') {
           continue;
       } else if (isdigit(c)) {
           boardPos += c - '0';
           truePos += c - '0';
       } else if (c == 'K') {

            truePos += 1;
           whiteKingBitset = bitset<6>(boardPos);
           for (size_t j = 0; j < 6; ++j)
               kingPositionBitset.set(j, whiteKingBitset[j]);
       } else if (c == 'k') {

        truePos += 1;
           blackKingBitset = bitset<6>(boardPos);
           for (size_t j = 0; j < 6; ++j)
               kingPositionBitset.set(kingPositionBitset.size() - 6 + j, blackKingBitset[j]);
       } else {
           if ((boardPos == 0 && get<0>(castlingRightsParsed)) ||
               (boardPos == 6 && get<1>(castlingRightsParsed)) ||
               (boardPos == 61 && get<3>(castlingRightsParsed)) ||
               (boardPos == 55 && get<2>(castlingRightsParsed))) {
               if (c == 'R' || c == 'r') {
                   insertSubBitset(pieceTypeBitset, fenEncoder.pieceMapping[string(1, c) + " with cr"], piecesFound);
                   piecesFound++;
               }
           }
           else if ((c == 'P' || c == 'p') && boardPos == epSquare) {
                
               insertSubBitset(pieceTypeBitset, fenEncoder.pieceMapping[string(1, c) + " with ep"], piecesFound);
               piecesFound++;
           } else {
               insertSubBitset(pieceTypeBitset, fenEncoder.pieceMapping[string(1, c)], piecesFound);
               piecesFound++;
           }
           cout << "set piece:  " << c << " is white: " << isupper(c)  << " at: " << boardPos << " ep index: " << epSquare << " True board pos: " << truePos <<"\n" ;
           occupancyBitset.set(boardPos, true);
           boardPos++;
           truePos += 1;
       }
   }
   return make_tuple(pieceTypeBitset, occupancyBitset, kingPositionBitset);
}
ChessBoard fenToChessBoard(const string& fen) {
   ChessBoard board;
   istringstream iss(fen);
   string boardPart;
   iss >> boardPart;
   int rank = 0, file = 0;
   for (char c : boardPart) {
       if (c == '/') {
           rank++;
           file = 0;
       } else if (isdigit(c)) {
           int emptyCount = c - '0';
           for (int i = 0; i < emptyCount; i++)
               board.squares[rank][file++] = ' ';
       } else {
           board.squares[rank][file++] = c;
       }
   }
   iss >> board.activeColor >> board.castlingRights >> board.enPassant
>> board.halfmoveClock >> board.fullmoveNumber;
   return board;
}
string chessBoardToFen(const ChessBoard& board) {
   string fen;
   for (int rank = 0; rank < 8; rank++) {
       int emptyCount = 0;
       for (int file = 0; file < 8; file++) {
           char piece = board.squares[rank][file];
           if (piece == ' ')
               emptyCount++;
           else {
               if (emptyCount > 0) {
                   fen += to_string(emptyCount);
                   emptyCount = 0;
               }
               fen.push_back(piece);
           }
       }
       if (emptyCount > 0)
           fen += to_string(emptyCount);
       if (rank != 7)
           fen.push_back('/');
   }
   fen += " ";
   fen.push_back(board.activeColor);
   fen += " " + board.castlingRights;
   fen += " " + board.enPassant;
   fen += " " + to_string(board.halfmoveClock);
   fen += " " + to_string(board.fullmoveNumber);
   return fen;
}
void printPieceTypeBitset(const bitset<120>& bits) {
   for (size_t i = 0; i < bits.size(); ++i)
       cout << bits[i];
   cout << "\n";
}
void printOccupancyBitset(const bitset<62>& bits) {
   for (size_t i = 0; i < bits.size(); ++i) {
       cout << bits[i];
       if ((i + 1) % 8 == 0)
           cout << "\n";
   }
   cout << "\n";
}
pair<bitset<6>, bitset<6>> splitKingPositions(const bitset<12>& kingSet) {
   bitset<6> whiteKing, blackKing;
   for (int i = 0; i < 6; i++) {
       whiteKing.set(i, kingSet[i]);
       blackKing.set(i, kingSet[i + 6]);
   }
   return make_pair(whiteKing, blackKing);
}

string generateFenPieceOrder(const bitset<62>& occupancy, const bitset<120>& pieceTypes, const bitset<12>& kingPositions) {
   int pieceIndex = 0, emptyCount = 0;
   string castlingRights;
   string fenPieces;
   FenPieceDecoder fenDecoder;
   bitset<4> piece;
   auto kings = splitKingPositions(kingPositions);
   int whiteKingIndex = kings.first.to_ulong();
   int blackKingIndex = kings.second.to_ulong();
   for (size_t i = 0; i < occupancy.size(); ++i) {
       if ((i + 1) % 8 == 0) {
           if (emptyCount != 0) {
               fenPieces += to_string(emptyCount);
               emptyCount = 0;
           }
           fenPieces += "/";
       }
       if (i == blackKingIndex)
           fenPieces.push_back('k');
       if (i == whiteKingIndex)
           fenPieces.push_back('K');
       if (occupancy.test(i)) {
           if (emptyCount != 0) {
               fenPieces += to_string(emptyCount);
               emptyCount = 0;
           }
           for (int j = 0; j < 4; ++j)
               piece.set(j, pieceTypes[pieceIndex * 4 + j]);
           if (fenDecoder.pieceMapping[piece] == "r with cr") {
               castlingRights += (i == 0 ? "K" : "Q");
               fenPieces.push_back('r');
           } else if (fenDecoder.pieceMapping[piece] == "R with cr") {
               castlingRights += (i == 61 ? "k" : "q");
               fenPieces.push_back('R');
           } else {
               fenPieces += fenDecoder.pieceMapping[piece];
           }
           pieceIndex++;
       } else {
           emptyCount++;
       }
   }

   string fen = fenPieces + " " + castlingRights;
   return fen;
}
int main() {
    // string fen = "rnbqkbnr/pppp1ppp/4p3/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e3 0 1";
    // string expected_fen = "rnbqkbnr/1pp1pppp/p7/3pP with ep3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3 (ep: 27, true: 35)";
    // string fen = "rnbqkbnr/1pp1pppp/p7/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3";
    string expected_fen = "rnbqkbnr/1pp1pppp/p7/4P3/2Pp with ep4/P7/1P1P1PPP/RNBQKBNR b KQkq c3 0 4 (ep: 33, true: 34)";
    string fen = "rnbqkbnr/1pp1pppp/p7/4P3/2Pp4/P7/1P1P1PPP/RNBQKBNR b KQkq c3 0 4";

    ChessBoard board = fenToChessBoard(fen);
    auto bitboards = encodeFenToBitboards(fen);
    // printPieceTypeBitset(get<0>(bitboards));
    // printOccupancyBitset(get<1>(bitboards));
    string decoeded_fen = generateFenPieceOrder(get<1>(bitboards), get<0>(bitboards), get<2>(bitboards));
    cout << "original: " << fen << "\n";
    cout << "expected: " << expected_fen << "\n";
    cout << "actual:   " <<decoeded_fen << "\n";
    return 0;
}
