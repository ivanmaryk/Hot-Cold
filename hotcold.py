
---

## 💻 Код на 7 языках

### 1. Python – `hotcold.py`

```python
#!/usr/bin/env python3
# hotcold.py - Игра "Горячо-холодно" на Python

import random
import sys
import os
import json
import time
from typing import List, Tuple, Optional

try:
    from colorama import init, Fore, Style
    init(autoreset=True)
    HAS_COLOR = True
except ImportError:
    HAS_COLOR = False
    class Fore:
        RED = GREEN = YELLOW = CYAN = MAGENTA = WHITE = RESET = ''
    Style = Fore

class GameState:
    def __init__(self, difficulty: int):
        self.difficulty = difficulty
        self.ranges = {1: (1, 50, 999), 2: (1, 100, 10), 3: (1, 200, 7)}
        self.low, self.high, self.max_attempts = self.ranges[difficulty]
        self.secret = random.randint(self.low, self.high)
        self.attempts = 0
        self.history: List[Tuple[int, str]] = []  # (число, "горячо"/"холодно")
        self.won = False
        self.record = self.load_record()

    def check_guess(self, guess: int) -> str:
        """Возвращает 'горячо' или 'холодно' на основе близости."""
        if guess == self.secret:
            self.won = True
            return "ПОБЕДА!"
        distance = abs(guess - self.secret)
        if distance <= 10:
            hint = "ГОРЯЧО"
        elif distance <= 25:
            hint = "ТЕПЛО"
        else:
            hint = "ХОЛОДНО"
        direction = "▲" if guess < self.secret else "▼"
        return f"{hint} {direction}"

    def add_attempt(self, guess: int, hint: str):
        self.attempts += 1
        self.history.append((guess, hint))

    def is_game_over(self) -> bool:
        return self.won or self.attempts >= self.max_attempts

    def save_record(self):
        record_file = "record.json"
        try:
            with open(record_file, 'r') as f:
                records = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError):
            records = {}
        key = f"{self.difficulty}_{self.low}_{self.high}"
        if key not in records or self.attempts < records[key]:
            records[key] = self.attempts
            with open(record_file, 'w') as f:
                json.dump(records, f, indent=2)
            return True
        return False

    def load_record(self) -> Optional[int]:
        try:
            with open("record.json", 'r') as f:
                records = json.load(f)
            key = f"{self.difficulty}_{self.low}_{self.high}"
            return records.get(key)
        except:
            return None

def display_history(history):
    print(Fore.CYAN + "\n--- История попыток ---")
    for i, (num, hint) in enumerate(history, 1):
        color = Fore.RED if "ГОРЯЧО" in hint else Fore.BLUE
        print(f"{i}: {num} -> {color}{hint}{Style.RESET_ALL}")
    print("------------------------")

def play_game():
    print(Fore.YELLOW + "🔥❄️ Добро пожаловать в Hot&Cold!")
    while True:
        print("\nВыберите уровень сложности:")
        print("1. Лёгкий (1-50, безлимит)")
        print("2. Средний (1-100, 10 попыток)")
        print("3. Сложный (1-200, 7 попыток)")
        print("4. Выйти")
        choice = input("Ваш выбор: ").strip()
        if choice == '4':
            print("До свидания!")
            break
        if choice not in ('1', '2', '3'):
            print(Fore.RED + "Неверный выбор.")
            continue
        diff = int(choice)
        game = GameState(diff)

        if game.record is not None:
            print(Fore.MAGENTA + f"Текущий рекорд: {game.record} попыток")
        else:
            print("Рекорда пока нет.")

        print(Fore.GREEN + f"Число загадано! Диапазон {game.low}-{game.high}")
        if game.max_attempts < 999:
            print(f"У вас {game.max_attempts} попыток.")
        else:
            print("Попытки не ограничены.")

        while not game.is_game_over():
            try:
                guess = int(input(f"Попытка {game.attempts+1}: Введите число: "))
            except ValueError:
                print(Fore.RED + "Введите целое число!")
                continue
            if guess < game.low or guess > game.high:
                print(Fore.RED + f"Число должно быть от {game.low} до {game.high}")
                continue

            hint = game.check_guess(guess)
            game.add_attempt(guess, hint)
            color = Fore.RED if "ГОРЯЧО" in hint else Fore.BLUE if "ХОЛОДНО" in hint else Fore.GREEN
            print(f"{color}{hint}{Style.RESET_ALL}")

            if game.won:
                print(Fore.GREEN + f"🎉 ПОБЕДА! Число {game.secret} угадано за {game.attempts} попыток!")
                if game.save_record():
                    print(Fore.CYAN + "🏆 Новый рекорд!")
                break
            elif game.attempts >= game.max_attempts:
                print(Fore.RED + f"😞 Вы исчерпали все попытки. Загаданное число: {game.secret}")
                break

        if game.history:
            display_history(game.history)

        again = input("Хотите сыграть ещё раз? (y/n): ").lower()
        if again != 'y':
            print("До свидания!")
            break

if __name__ == "__main__":
    # Аргументы командной строки: --auto [difficulty] для автоматической игры
    if len(sys.argv) > 1 and sys.argv[1] == "--auto":
        diff = int(sys.argv[2]) if len(sys.argv) > 2 else 2
        game = GameState(diff)
        print(f"Автоигра на уровне {diff}")
        low, high = game.low, game.high
        # Простая бинарная стратегия
        left, right = low, high
        while not game.is_game_over():
            guess = (left + right) // 2
            print(f"Компьютер пробует: {guess}")
            hint = game.check_guess(guess)
            game.add_attempt(guess, hint)
            if game.won:
                print(f"Угадал за {game.attempts} попыток!")
                break
            if "▲" in hint:
                left = guess + 1
            else:
                right = guess - 1
            time.sleep(0.5)
        if not game.won:
            print("Компьютер не справился :(")
    else:
        play_game()
