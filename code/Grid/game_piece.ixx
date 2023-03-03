export module game_piece;

import <memory>;

export class GamePiece {
public:
  virtual ~GamePiece() = default;
  virtual std::unique_ptr<GamePiece> clone() = 0;
};