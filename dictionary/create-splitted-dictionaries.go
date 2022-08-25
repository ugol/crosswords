package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
)

func splitDictionaries(path string) {
	file, err := os.Open(path)
	if err != nil {
		fmt.Println(err)
	}
	defer func(file *os.File) {
		err := file.Close()
		if err != nil {
			fmt.Println(err)
		}
	}(file)

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		word := scanner.Text()
		l := len(word)
		//err := os.WriteFile(strconv.Itoa(l)+".txt", word, 0666)

		f, err := os.OpenFile(strconv.Itoa(l)+".txt",
			os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
		if err != nil {
			log.Println(err)
		}
		if _, err := f.WriteString(word + "\n"); err != nil {
			log.Println(err)
		}

		if err != nil {
			log.Print(err)
		}
	}

}

func main() {

	splitDictionaries("./full-ita.txt")
	//name := strconv.Itoa(i)

}
