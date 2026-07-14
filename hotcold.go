// hotcold.go - Игра "Горячо-холодно" на Go
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"math/rand"
	"os"
	"strconv"
	"strings"
	"time"
)

type GameState struct {
	Difficulty   int
	Low          int
	High         int
	MaxAttempts  int
	Secret       int
	Attempts     int
	History      []HistoryEntry
	Won          bool
	Record       *int
}

type HistoryEntry struct {
	Guess int
	Hint  string
}

func NewGameState(diff int) *GameState {
	ranges := map[int][3]int{1: {1, 50, 999}, 2: {1, 100, 10}, 3: {1, 200, 7}}
	low, high, max := ranges[diff][0], ranges[diff][1], ranges[diff][2]
	rand.Seed(time.Now().UnixNano())
	return &GameState{
		Difficulty:  diff,
		Low:         low,
		High:        high,
		MaxAttempts: max,
		Secret:      rand.Intn(high-low+1) + low,
		Attempts:    0,
		History:     []HistoryEntry{},
		Won:         false,
		Record:      loadRecord(diff),
	}
}

func (g *GameState) CheckGuess(guess int) string {
	if guess == g.Secret {
		g.Won = true
		return "ПОБЕДА!"
	}
	dist := abs(guess - g.Secret)
	var hint string
	if dist <= 10 {
		hint = "ГОРЯЧО"
	} else if dist <= 25 {
		hint = "ТЕПЛО"
	} else {
		hint = "ХОЛОДНО"
	}
	direction := "▲"
	if guess > g.Secret {
		direction = "▼"
	}
	return fmt.Sprintf("%s %s", hint, direction)
}

func (g *GameState) AddAttempt(guess int, hint string) {
	g.Attempts++
	g.History = append(g.History, HistoryEntry{Guess: guess, Hint: hint})
}

func (g *GameState) IsGameOver() bool {
	return g.Won || g.Attempts >= g.MaxAttempts
}

func (g *GameState) SaveRecord() bool {
	records := loadAllRecords()
	key := fmt.Sprintf("%d_%d_%d", g.Difficulty, g.Low, g.High)
	if val, ok := records[key]; !ok || g.Attempts < val {
		records[key] = g.Attempts
		saveAllRecords(records)
		return true
	}
	return false
}

func loadRecord(diff int) *int {
	records := loadAllRecords()
	key := fmt.Sprintf("%d_%d_%d", diff, map[int][3]int{1: {1, 50, 999}, 2: {1, 100, 10}, 3: {1, 200, 7}}[diff][0],
		map[int][3]int{1: {1, 50, 999}, 2: {1, 100, 10}, 3: {1, 200, 7}}[diff][1])
	if val, ok := records[key]; ok {
		return &val
	}
	return nil
}

func loadAllRecords() map[string]int {
	file, err := os.Open("records.json")
	if err != nil {
		return make(map[string]int)
	}
	defer file.Close()
	var records map[string]int
	decoder := json.NewDecoder(file)
	if err := decoder.Decode(&records); err != nil {
		return make(map[string]int)
	}
	return records
}

func saveAllRecords(records map[string]int) {
	file, _ := os.Create("records.json")
	defer file.Close()
	encoder := json.NewEncoder(file)
	encoder.SetIndent("", "  ")
	encoder.Encode(records)
}

func abs(x int) int {
	if x < 0 {
		return -x
	}
	return x
}

func displayHistory(history []HistoryEntry) {
	fmt.Println("\x1b[36m--- История попыток ---\x1b[0m")
	for i, h := range history {
		color := "\x1b[31m"
		if strings.Contains(h.Hint, "ХОЛОДНО") {
			color = "\x1b[34m"
		}
		fmt.Printf("%d: %d -> %s%s\x1b[0m\n", i+1, h.Guess, color, h.Hint)
	}
	fmt.Println("------------------------")
}

func readLine() string {
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Scan()
	return strings.TrimSpace(scanner.Text())
}

func playGame() {
	fmt.Println("\x1b[33m🔥❄️ Добро пожаловать в Hot&Cold!\x1b[0m")
	for {
		fmt.Println("\nВыберите уровень сложности:")
		fmt.Println("1. Лёгкий (1-50, безлимит)")
		fmt.Println("2. Средний (1-100, 10 попыток)")
		fmt.Println("3. Сложный (1-200, 7 попыток)")
		fmt.Println("4. Выйти")
		fmt.Print("Ваш выбор: ")
		choice := readLine()
		if choice == "4" {
			fmt.Println("До свидания!")
			break
		}
		if choice != "1" && choice != "2" && choice != "3" {
			fmt.Println("\x1b[31mНеверный выбор.\x1b[0m")
			continue
		}
		diff, _ := strconv.Atoi(choice)
		game := NewGameState(diff)
		if game.Record != nil {
			fmt.Printf("\x1b[35mТекущий рекорд: %d попыток\x1b[0m\n", *game.Record)
		} else {
			fmt.Println("Рекорда пока нет.")
		}
		fmt.Printf("\x1b[32mЧисло загадано! Диапазон %d-%d\x1b[0m\n", game.Low, game.High)
		if game.MaxAttempts < 999 {
			fmt.Printf("У вас %d попыток.\n", game.MaxAttempts)
		} else {
			fmt.Println("Попытки не ограничены.")
		}

		for !game.IsGameOver() {
			fmt.Printf("Попытка %d: Введите число: ", game.Attempts+1)
			input := readLine()
			guess, err := strconv.Atoi(input)
			if err != nil {
				fmt.Println("\x1b[31mВведите целое число!\x1b[0m")
				continue
			}
			if guess < game.Low || guess > game.High {
				fmt.Printf("\x1b[31mЧисло должно быть от %d до %d\x1b[0m\n", game.Low, game.High)
				continue
			}
			hint := game.CheckGuess(guess)
			game.AddAttempt(guess, hint)
			color := "\x1b[31m"
			if strings.Contains(hint, "ХОЛОДНО") {
				color = "\x1b[34m"
			} else if strings.Contains(hint, "ПОБЕДА") {
				color = "\x1b[32m"
			}
			fmt.Printf("%s%s\x1b[0m\n", color, hint)

			if game.Won {
				fmt.Printf("\x1b[32m🎉 ПОБЕДА! Число %d угадано за %d попыток!\x1b[0m\n", game.Secret, game.Attempts)
				if game.SaveRecord() {
					fmt.Println("\x1b[36m🏆 Новый рекорд!\x1b[0m")
				}
				break
			} else if game.Attempts >= game.MaxAttempts {
				fmt.Printf("\x1b[31m😞 Вы исчерпали все попытки. Загаданное число: %d\x1b[0m\n", game.Secret)
				break
			}
		}
		if len(game.History) > 0 {
			displayHistory(game.History)
		}
		fmt.Print("Хотите сыграть ещё раз? (y/n): ")
		again := readLine()
		if again != "y" {
			fmt.Println("До свидания!")
			break
		}
	}
}

func main() {
	// Аргументы командной строки: --auto [difficulty]
	if len(os.Args) > 1 && os.Args[1] == "--auto" {
		diff := 2
		if len(os.Args) > 2 {
			diff, _ = strconv.Atoi(os.Args[2])
		}
		game := NewGameState(diff)
		fmt.Printf("Автоигра на уровне %d\n", diff)
		left, right := game.Low, game.High
		for !game.IsGameOver() {
			guess := (left + right) / 2
			fmt.Printf("Компьютер пробует: %d\n", guess)
			hint := game.CheckGuess(guess)
			game.AddAttempt(guess, hint)
			if game.Won {
				fmt.Printf("Угадал за %d попыток!\n", game.Attempts)
				break
			}
			if strings.Contains(hint, "▲") {
				left = guess + 1
			} else {
				right = guess - 1
			}
			time.Sleep(500 * time.Millisecond)
		}
		if !game.Won {
			fmt.Println("Компьютер не справился :(")
		}
	} else {
		playGame()
	}
}
