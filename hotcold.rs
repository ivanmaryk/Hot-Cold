// hotcold.rs - Игра "Горячо-холодно" на Rust
// Для запуска требуется Cargo.toml с зависимостями:
// [dependencies]
// rand = "0.8"
// serde_json = "1.0"

use std::io::{self, Write, BufRead};
use std::fs;
use std::collections::HashMap;
use rand::Rng;
use serde_json;

struct Game {
    difficulty: u8,
    low: i32,
    high: i32,
    max_attempts: i32,
    secret: i32,
    attempts: i32,
    history: Vec<(i32, String)>,
    won: bool,
    record: Option<i32>,
}

impl Game {
    fn new(diff: u8) -> Self {
        let (low, high, max_attempts) = match diff {
            1 => (1, 50, 999),
            2 => (1, 100, 10),
            3 => (1, 200, 7),
            _ => (1, 100, 10),
        };
        let mut rng = rand::thread_rng();
        let secret = rng.gen_range(low..=high);
        let mut game = Self {
            difficulty: diff,
            low,
            high,
            max_attempts,
            secret,
            attempts: 0,
            history: Vec::new(),
            won: false,
            record: None,
        };
        game.load_record();
        game
    }

    fn check_guess(&mut self, guess: i32) -> String {
        if guess == self.secret {
            self.won = true;
            return "ПОБЕДА!".to_string();
        }
        let dist = (guess - self.secret).abs();
        let hint = if dist <= 10 { "ГОРЯЧО" } else if dist <= 25 { "ТЕПЛО" } else { "ХОЛОДНО" };
        let dir = if guess < self.secret { " ▲" } else { " ▼" };
        format!("{}{}", hint, dir)
    }

    fn add_attempt(&mut self, guess: i32, hint: String) {
        self.attempts += 1;
        self.history.push((guess, hint));
    }

    fn is_game_over(&self) -> bool {
        self.won || self.attempts >= self.max_attempts
    }

    fn save_record(&mut self) -> bool {
        let key = format!("{}_{}_{}", self.difficulty, self.low, self.high);
        let mut records: HashMap<String, i32> = HashMap::new();
        if let Ok(data) = fs::read_to_string("records.json") {
            if let Ok(map) = serde_json::from_str(&data) {
                records = map;
            }
        }
        if let Some(&old) = records.get(&key) {
            if self.attempts >= old {
                return false;
            }
        }
        records.insert(key, self.attempts);
        let _ = fs::write("records.json", serde_json::to_string_pretty(&records).unwrap());
        true
    }

    fn load_record(&mut self) {
        let key = format!("{}_{}_{}", self.difficulty, self.low, self.high);
        if let Ok(data) = fs::read_to_string("records.json") {
            if let Ok(records) = serde_json::from_str::<HashMap<String, i32>>(&data) {
                self.record = records.get(&key).copied();
            }
        }
    }

    fn display_history(&self) {
        println!("\x1b[36m--- История попыток ---\x1b[0m");
        for (i, (guess, hint)) in self.history.iter().enumerate() {
            let color = if hint.contains("ГОРЯЧО") { "\x1b[31m" } else { "\x1b[34m" };
            println!("{}: {} -> {}{}\x1b[0m", i+1, guess, color, hint);
        }
        println!("------------------------");
    }
}

fn read_line() -> String {
    let stdin = io::stdin();
    let mut line = String::new();
    stdin.lock().read_line(&mut line).expect("Ошибка ввода");
    line.trim().to_string()
}

fn play_game() {
    println!("\x1b[33m🔥❄️ Добро пожаловать в Hot&Cold!\x1b[0m");
    loop {
        println!("\nВыберите уровень сложности:");
        println!("1. Лёгкий (1-50, безлимит)");
        println!("2. Средний (1-100, 10 попыток)");
        println!("3. Сложный (1-200, 7 попыток)");
        println!("4. Выйти");
        print!("Ваш выбор: ");
        io::stdout().flush().unwrap();
        let choice = read_line();
        if choice == "4" {
            println!("До свидания!");
            break;
        }
        let diff: u8 = match choice.parse() {
            Ok(d) if d >= 1 && d <= 3 => d,
            _ => {
                println!("\x1b[31mНеверный выбор.\x1b[0m");
                continue;
            }
        };

        let mut game = Game::new(diff);

        if let Some(rec) = game.record {
            println!("\x1b[35mТекущий рекорд: {} попыток\x1b[0m", rec);
        } else {
            println!("Рекорда пока нет.");
        }
        println!("\x1b[32mЧисло загадано! Диапазон {}-{}\x1b[0m", game.low, game.high);
        if game.max_attempts < 999 {
            println!("У вас {} попыток.", game.max_attempts);
        } else {
            println!("Попытки не ограничены.");
        }

        while !game.is_game_over() {
            print!("Попытка {}: Введите число: ", game.attempts+1);
            io::stdout().flush().unwrap();
            let input = read_line();
            let guess: i32 = match input.parse() {
                Ok(n) => n,
                Err(_) => {
                    println!("\x1b[31mВведите целое число!\x1b[0m");
                    continue;
                }
            };
            if guess < game.low || guess > game.high {
                println!("\x1b[31mЧисло должно быть от {} до {}\x1b[0m", game.low, game.high);
                continue;
            }
            let hint = game.check_guess(guess);
            game.add_attempt(guess, hint.clone());
            let color = if hint.contains("ГОРЯЧО") { "\x1b[31m" } else if hint.contains("ХОЛОДНО") { "\x1b[34m" } else { "\x1b[32m" };
            println!("{}{}\x1b[0m", color, hint);

            if game.won {
                println!("\x1b[32m🎉 ПОБЕДА! Число {} угадано за {} попыток!\x1b[0m", game.secret, game.attempts);
                if game.save_record() {
                    println!("\x1b[36m🏆 Новый рекорд!\x1b[0m");
                }
                break;
            } else if game.attempts >= game.max_attempts {
                println!("\x1b[31m😞 Вы исчерпали все попытки. Загаданное число: {}\x1b[0m", game.secret);
                break;
            }
        }
        game.display_history();
        print!("Хотите сыграть ещё раз? (y/n): ");
        io::stdout().flush().unwrap();
        let again = read_line();
        if again.to_lowercase() != "y" {
            println!("До свидания!");
            break;
        }
    }
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() > 1 && args[1] == "--auto" {
        let diff: u8 = if args.len() > 2 { args[2].parse().unwrap_or(2) } else { 2 };
        let mut game = Game::new(diff);
        println!("Автоигра на уровне {}", diff);
        let mut left = game.low;
        let mut right = game.high;
        while !game.is_game_over() {
            let guess = (left + right) / 2;
            println!("Компьютер пробует: {}", guess);
            let hint = game.check_guess(guess);
            game.add_attempt(guess, hint.clone());
            if game.won {
                println!("Угадал за {} попыток!", game.attempts);
                break;
            }
            if hint.contains("▲") {
                left = guess + 1;
            } else {
                right = guess - 1;
            }
            std::thread::sleep(std::time::Duration::from_millis(500));
        }
        if !game.won {
            println!("Компьютер не справился :(");
        }
    } else {
        play_game();
    }
}
