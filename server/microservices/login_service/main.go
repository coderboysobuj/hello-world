package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"time"
)

type LoginRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

type LoginResponse struct {
	Success bool   `json:"success"`
	Message string `json:"message,omitempty"`
	Token   string `json:"token,omitempty"`
}

func loginHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req LoginRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid request body", http.StatusBadRequest)
		return
	}

	// Basic validation (In-Memory Mock)
	if req.Username == "" || req.Password == "" {
		sendResponse(w, false, "Username and password required", "")
		return
	}

	if req.Password != "password123" {
		sendResponse(w, false, "Invalid credentials", "")
		return
	}

	// Generate a mock Session Token
	token := fmt.Sprintf("TOKEN_%s_%d", req.Username, time.Now().Unix())
	
	sendResponse(w, true, "Login successful", token)
	log.Printf("User '%s' logged in successfully. Token: %s", req.Username, token)
}

func sendResponse(w http.ResponseWriter, success bool, message, token string) {
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(LoginResponse{
		Success: success,
		Message: message,
		Token:   token,
	})
}

func main() {
	http.HandleFunc("/login", loginHandler)
	
	port := "8080"
	log.Printf("Starting Login Service on port %s...", port)
	if err := http.ListenAndServe(":"+port, nil); err != nil {
		log.Fatalf("Server failed: %v", err)
	}
}
