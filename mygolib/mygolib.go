package main

import "C"
import "time"

//export BlockInSelect
func BlockInSelect() int {
	select {
	case <-time.After(10 * time.Second):
		return 3
	}
}

func main() {

}
