
package main

import (
	"fmt"
	"os"
	"path/filepath"
	"io"
	"time"
	"flag"
)

const (
	BLOCK_SIZE = 512
	FILE_PATH_INDEX = 13
)

type Options struct {
	printSize         bool
	printAllFiles     bool
	printOnlyFullsize bool
	printFullsize     bool
	blockSize         bool
	fromFile          bool
}

func processFile(filePath string, totalSize *uint64, options *Options) {
	fileInfo, err := os.Stat(filePath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error processing file: %s - %v\n", filePath, err)
		return
	}

	fileSize := uint64(fileInfo.Size())
	*totalSize += fileSize

	if options.printSize {
		if options.blockSize {
			fmt.Printf("%d\t", (fileSize + BLOCK_SIZE - 1) / BLOCK_SIZE)
		} else {
			fmt.Printf("%d\t", fileSize)
		}
	}

	if !options.printOnlyFullsize {
		time.Sleep(50 * time.Millisecond)
		fmt.Print(filePath)
	}

	if options.printAllFiles {
		fmt.Printf("\t%d %s", fileSize, func() string {
			if options.blockSize {
				return "blocks"
			}
			return "bytes"
		}())
	}

	if !options.printOnlyFullsize {
		fmt.Println()
	}
}

func processDirectory(path string, totalSize *uint64, options *Options) {
	err := filepath.Walk(path, func(entry string, info os.FileInfo, err error) error {
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error processing: %s - %v\n", entry, err)
			return nil
		}

		if !info.IsDir() {
			processFile(entry, totalSize, options)
		}
		return nil
	})

	if err != nil {
		fmt.Fprintf(os.Stderr, "Error walking directory: %v\n", err)
	}
}

func processPaths(paths []string, options *Options, totalSize *uint64) {
	for _, targetPath := range paths {
		if _, err := os.Stat(targetPath); os.IsNotExist(err) {
			fmt.Fprintf(os.Stderr, "Path does not exist: %s\n", targetPath)
			continue
		}
		processDirectory(targetPath, totalSize, options)
	}
}

func processFileFromOptions(filePath string, totalSize *uint64, options *Options) {
	file, err := os.Open(filePath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Unable to open file: %s\n", filePath)
		os.Exit(1)
	}
	defer file.Close()

	var line string
	for {
		_, err := fmt.Fscanf(file, "%s", &line)
		if err == io.EOF {
			break
		}
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error reading from file: %s\n", err)
			break
		}
		processDirectory(line, totalSize, options)
	}
}

func main() {
	var totalSize uint64
	options := &Options{
		blockSize: true,
	}

	// Parse flags
	flag.BoolVar(&options.printSize, "b", false, "Print size in bytes")
	flag.BoolVar(&options.printAllFiles, "a", false, "Print all file details")
	flag.BoolVar(&options.printFullsize, "c", false, "Print full size")
	flag.BoolVar(&options.printOnlyFullsize, "s", false, "Print only the full size")

	filesFrom := flag.String("files-from", "", "Read file paths from file")
	flag.Parse()

	// If `--files-from` is set
	if *filesFrom != "" {
		options.fromFile = true
		processFileFromOptions(*filesFrom, &totalSize, options)
	} else {
		// Process file paths from arguments
		paths := flag.Args()
		processPaths(paths, options, &totalSize)
	}

	// Print total size if needed
	if options.printOnlyFullsize || options.printFullsize {
		fmt.Printf("Total size: %d %s\n", totalSize, func() string {
			if options.blockSize {
				return "blocks"
			}
			return "bytes"
		}())
	}
}
