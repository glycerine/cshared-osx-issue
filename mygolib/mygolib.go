package main

import (
	"fmt"
	"os"
	"os/signal"
)

import "C"

//export BlockInSelect
func BlockInSelect() int {

	ctrlC_Chan := make(chan os.Signal, 5)
	signal.Notify(ctrlC_Chan, os.Interrupt)

	count := 0
	fmt.Printf("mylib.go: in BlockInSelect(): about to select on ctrlC_Chan\n")
	for {
		select {
		case <-ctrlC_Chan:
			fmt.Printf("\n\n  I see ctrl-c !!\n\n")
			count++
			if count >= 2 {
				return 0
			}
		}
	}
	return 0
}

func main() {

}
