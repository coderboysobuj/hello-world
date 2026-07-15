package main

import (
	"fmt"
	"log"
	"net/http"
)

func main() {
	http.HandleFunc("/login", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, "Login Service Active")
	})
	log.Println("Login Service starting on :8081")
	log.Fatal(http.ListenAndServe(":8081", nil))
}
