package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

var words [26][]string

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

	for i := 2; i < 26; i++ {
		name := strconv.Itoa(i)
		words[i], _ = createDictionary("./dictionary/" + name + ".txt")
	}

	words := findAllMatchingWords(os.Args[1])
	fmt.Println(words)
	fmt.Println(len(words))

	/*
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
	*/

}
