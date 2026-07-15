package main

import (
	"fmt"
	"log"
	"net/http"
)

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, "API Gateway Active")
	})
	log.Println("Gateway starting on :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
