package main

import "C"

//export BlockInSelect
func BlockInSelect() int {
	select {}
}

func main() {

}
