export module game_board;

import grid;
import game_piece;
import <concepts>;

export template <typename T> requires std::derived_from<T, GamePiece>
class GameBoard :public Grid<T> {
public:
	explicit GameBoard(size_t width = Grid<T>::DefaultWidth,
		size_t height = Grid<T>::DefaultHeight);
	void move(size_t xSrc, size_t ySrc, size_t xDest, size_t yDest)
		requires std::movable<T>;
};

template <typename T> requires std::derived_from<T, GamePiece>
// template <std::derived_from<GamePiece> T>
GameBoard<T>::GameBoard(size_t width, size_t height)
	: Grid<T> { width, height } {}

template <typename T> requires std::derived_from<T, GamePiece>
// template <std::derived_from<GamePiece> T>
void GameBoard<T>::move(size_t xSrc, size_t ySrc, size_t xDest, size_t yDest) 
	requires std::movable<T> {
	Grid<T>::at(xDest, yDest) = std::move(Grid<T>::at(xSrc, ySrc));
	Grid<T>::at(xSrc, ySrc).reset();
}