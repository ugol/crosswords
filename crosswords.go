package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

var words [25][]string

type Grid struct {
	grid [][]byte
	h    []Word
	v    []Word
}

type Word struct {
	x, y uint8
	word string
}

func NewGrid(grid []string) *Grid {

	g := &Grid{
		grid: make([][]byte, len(grid)),
		h:    nil,
		v:    nil,
	}

	for i, l := range grid {
		g.grid[i] = []byte(l)
	}

	rows := len(g.grid)
	columns := len(g.grid[0])

	for row := 0; row < rows; row++ {
		for col := 0; col < columns; col++ {
			if g.grid[row][col] != '*' {

				if (col == 0 || g.grid[row][col-1] == '*') && (col < columns-1 && g.grid[row][col+1] != '*') {

					w := make([]byte, columns-col)

					for c := col; c < columns && g.grid[row][c] != '*'; c++ {
						w[c-col] = g.grid[row][c]
					}
					g.h = append(g.h, Word{x: uint8(row), y: uint8(col), word: string(w)})
				}

				if (row == 0 || g.grid[row-1][col] == '*') && (row < rows-1 && g.grid[row+1][col] != '*') {

					w := make([]byte, rows-row)

					for r := row; r < rows && g.grid[r][col] != '*'; r++ {
						w[r-row] = g.grid[r][col]
					}

					g.v = append(g.v, Word{x: uint8(row), y: uint8(col), word: string(w)})
				}

			}
		}
	}

	return g
}

func (g *Grid) crossingWords(i int) []string {
	return nil
}

func (g *Grid) print() {

	rows := len(g.grid)
	columns := len(g.grid[0])

	fmt.Printf("Rows: %v\n", rows)
	fmt.Printf("Columns: %v\n", columns)

	fmt.Println()

	for row := 0; row < rows; row++ {
		for col := 0; col < columns; col++ {
			//fmt.Printf("%c (%v,%v) ", g.grid[row][col], row, col)
			fmt.Printf("%c", g.grid[row][col])
		}
		fmt.Println()
	}

	fmt.Println()
	fmt.Printf("Horizontal: %v\n", g.h)
	fmt.Printf("Vertical: %v\n", g.v)

}

func matchWord(original string, pattern string) (b bool) {

	if len(original) != len(pattern) {
		return false
	}

	for i, b := range []byte(pattern) {

		if b != '_' {
			if b != original[i] {
				return false
			}
		}
	}

	return true
}

func findAllMatchingWords(pattern string) []string {

	var matched []string
	p := strings.ToLower(pattern)
	for _, word := range words[len(p)] {

		if matchWord(word, p) {
			matched = append(matched, word)
		}
	}

	return matched
}

func createDictionary(path string) ([]string, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	var lines []string
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	return lines, scanner.Err()
}

func main() {

	for i := 2; i < 25; i++ {
		name := strconv.Itoa(i)
		words[i], _ = createDictionary("./words/" + name + ".txt")
	}

	words := findAllMatchingWords(os.Args[1])

	fmt.Println(words)
	fmt.Println(len(words))

	grid := NewGrid([]string{
		"CATENA*B",
		"ACROBATA",
		"**IL**AR",
		"ETTO*UN*",
		"*EA*OMAR",
		"UL**PA**",
		"SERPENTE",
		"A*ATLETA"})
	grid.print()
}
