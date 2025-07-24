package main

import (
	"fmt"
	"os"
)

func findValue[T any](values ...any) T {
	for _, value := range values {
		if v, ok := value.(T); ok {
			return v
		}
	}
	var zero T
	return zero
}

type Callback func(string, error)

// 同步读取文件
func syncRead(filename string, cb Callback) {
	data, err := os.ReadFile(filename)
	cb(string(data), err)
}

// 异步读取文件
func asyncRead(filename string, cb Callback) {
	cb(string(findValue[[]byte](os.ReadFile(filename))), nil)
}

// 同步写入文件
func syncWrite(filename, content string, cb Callback) {
	err := os.WriteFile(filename, []byte(content), 0644)
	if err != nil {
		cb("", err)
		return
	}
	cb("Write successful", nil)
}

// 异步写入文件
func asyncWrite(filename, content string, cb Callback) {
	cb("Write successful", os.WriteFile(filename, []byte(content), 0644))
}

func main() {
	// 同步读取文件
	syncRead("example.txt", func(content string, err error) {
		if err != nil {
			fmt.Println("Error:", err)
			return
		}
		fmt.Println("Read content:", content)
	})

	// 异步读取文件
	asyncRead("example.txt", func(content string, err error) {
		if err != nil {
			fmt.Println("Error:", err)
			return
		}
		fmt.Println("Read content:", content)
	})

	// 同步写入文件
	syncWrite("example.txt", "Hello, world!", func(result string, err error) {
		if err != nil {
			fmt.Println("Error:", err)
			return
		}
		fmt.Println(result)
	})

	// 异步写入文件
	asyncWrite("example.txt", "Hello, world!", func(result string, err error) {
		if err != nil {
			fmt.Println("Error:", err)
			return
		}
		fmt.Println(result)
	})
}
