// hotcold.cpp - Игра "Горячо-холодно" на C++17
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <variant>
#include <memory>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

struct HistoryEntry {
    int guess;
    std::string hint;
};

class Game {
private:
    int difficulty;
    int low, high, maxAttempts;
    int secret;
    int attempts;
    std::vector<HistoryEntry> history;
    bool won;
    std::unique_ptr<int> record;

    std::mt19937 rng;

public:
    Game(int diff) : difficulty(diff), attempts(0), won(false), rng(std::random_device{}()) {
        switch(diff) {
            case 1: low=1; high=50; maxAttempts=999; break;
            case 2: low=1; high=100; maxAttempts=10; break;
            case 3: low=1; high=200; maxAttempts=7; break;
            default: low=1; high=100; maxAttempts=10; break;
        }
        std::uniform_int_distribution<int> dist(low, high);
        secret = dist(rng);
        loadRecord();
    }

    std::string checkGuess(int guess) {
        if (guess == secret) {
            won = true;
            return "ПОБЕДА!";
        }
        int dist = std::abs(guess - secret);
        std::string hint;
        if (dist <= 10) hint = "ГОРЯЧО";
        else if (dist <= 25) hint = "ТЕПЛО";
        else hint = "ХОЛОДНО";
        hint += (guess < secret) ? " ▲" : " ▼";
        return hint;
    }

    void addAttempt(int guess, const std::string& hint) {
        attempts++;
        history.push_back({guess, hint});
    }

    bool isGameOver() const { return won || attempts >= maxAttempts; }

    void saveRecord() {
        std::ifstream in("records.json");
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();
        // Простой JSON-парсинг (для демонстрации)
        // В реальности лучше использовать библиотеку, но здесь упростим
        std::string key = std::to_string(difficulty) + "_" + std::to_string(low) + "_" + std::to_string(high);
        // Если файл пуст, создадим пустой объект
        std::string newContent;
        if (content.empty()) {
            newContent = "{}";
        } else {
            newContent = content;
        }
        // Попытка найти ключ и обновить
        size_t pos = newContent.find("\"" + key + "\"");
        if (pos != std::string::npos) {
            // Найдем значение после двоеточия
            size_t colon = newContent.find(':', pos);
            size_t end = newContent.find(',', colon);
            if (end == std::string::npos) end = newContent.find('}', colon);
            std::string valStr = newContent.substr(colon+1, end-colon-1);
            int oldVal = std::stoi(valStr);
            if (attempts < oldVal) {
                // Заменяем
                newContent.replace(colon+1, end-colon-1, std::to_string(attempts));
            } else {
                return; // не улучшили
            }
        } else {
            // Добавляем новый ключ
            // Удаляем завершающую скобку
            size_t brace = newContent.rfind('}');
            if (brace != std::string::npos) {
                if (newContent.size() > 2) { // есть другие поля
                    newContent.insert(brace, ",");
                }
                newContent.insert(brace, "\"" + key + "\":" + std::to_string(attempts));
            }
        }
        std::ofstream out("records.json");
        out << newContent;
        out.close();
    }

    void loadRecord() {
        std::ifstream in("records.json");
        if (!in) return;
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();
        std::string key = std::to_string(difficulty) + "_" + std::to_string(low) + "_" + std::to_string(high);
        size_t pos = content.find("\"" + key + "\"");
        if (pos != std::string::npos) {
            size_t colon = content.find(':', pos);
            size_t end = content.find(',', colon);
            if (end == std::string::npos) end = content.find('}', colon);
            std::string valStr = content.substr(colon+1, end-colon-1);
            record = std::make_unique<int>(std::stoi(valStr));
        }
    }

    void displayHistory() const {
        std::cout << CYAN << "--- История попыток ---" << RESET << std::endl;
        for (size_t i = 0; i < history.size(); ++i) {
            const auto& h = history[i];
            std::string color = (h.hint.find("ГОРЯЧО") != std::string::npos) ? RED : BLUE;
            std::cout << i+1 << ": " << h.guess << " -> " << color << h.hint << RESET << std::endl;
        }
        std::cout << "------------------------" << std::endl;
    }

    int getAttempts() const { return attempts; }
    int getSecret() const { return secret; }
    bool hasRecord() const { return record != nullptr; }
    int getRecord() const { return *record; }
    int getLow() const { return low; }
    int getHigh() const { return high; }
    int getMaxAttempts() const { return maxAttempts; }
    bool isWon() const { return won; }
};

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void playGame() {
    std::cout << YELLOW << "🔥❄️ Добро пожаловать в Hot&Cold!" << RESET << std::endl;
    while (true) {
        std::cout << "\nВыберите уровень сложности:" << std::endl;
        std::cout << "1. Лёгкий (1-50, безлимит)" << std::endl;
        std::cout << "2. Средний (1-100, 10 попыток)" << std::endl;
        std::cout << "3. Сложный (1-200, 7 попыток)" << std::endl;
        std::cout << "4. Выйти" << std::endl;
        std::cout << "Ваш выбор: ";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "4") {
            std::cout << "До свидания!" << std::endl;
            break;
        }
        if (choice != "1" && choice != "2" && choice != "3") {
            std::cout << RED << "Неверный выбор." << RESET << std::endl;
            continue;
        }
        int diff = std::stoi(choice);
        Game game(diff);

        if (game.hasRecord()) {
            std::cout << MAGENTA << "Текущий рекорд: " << game.getRecord() << " попыток" << RESET << std::endl;
        } else {
            std::cout << "Рекорда пока нет." << std::endl;
        }
        std::cout << GREEN << "Число загадано! Диапазон " << game.getLow() << "-" << game.getHigh() << RESET << std::endl;
        if (game.getMaxAttempts() < 999) {
            std::cout << "У вас " << game.getMaxAttempts() << " попыток." << std::endl;
        } else {
            std::cout << "Попытки не ограничены." << std::endl;
        }

        while (!game.isGameOver()) {
            std::cout << "Попытка " << game.getAttempts()+1 << ": Введите число: ";
            std::string input;
            std::getline(std::cin, input);
            int guess;
            try {
                guess = std::stoi(input);
            } catch (...) {
                std::cout << RED << "Введите целое число!" << RESET << std::endl;
                continue;
            }
            if (guess < game.getLow() || guess > game.getHigh()) {
                std::cout << RED << "Число должно быть от " << game.getLow() << " до " << game.getHigh() << RESET << std::endl;
                continue;
            }
            std::string hint = game.checkGuess(guess);
            game.addAttempt(guess, hint);
            std::string color = (hint.find("ГОРЯЧО") != std::string::npos) ? RED :
                                (hint.find("ХОЛОДНО") != std::string::npos) ? BLUE : GREEN;
            std::cout << color << hint << RESET << std::endl;

            if (game.isWon()) {
                std::cout << GREEN << "🎉 ПОБЕДА! Число " << game.getSecret() << " угадано за " << game.getAttempts() << " попыток!" << RESET << std::endl;
                game.saveRecord();
                if (game.hasRecord() && game.getAttempts() == game.getRecord()) // только что обновили
                    std::cout << CYAN << "🏆 Новый рекорд!" << RESET << std::endl;
                break;
            } else if (game.isGameOver()) {
                std::cout << RED << "😞 Вы исчерпали все попытки. Загаданное число: " << game.getSecret() << RESET << std::endl;
                break;
            }
        }
        game.displayHistory();
        std::cout << "Хотите сыграть ещё раз? (y/n): ";
        std::string again;
        std::getline(std::cin, again);
        if (again != "y") {
            std::cout << "До свидания!" << std::endl;
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--auto") {
        int diff = 2;
        if (argc > 2) diff = std::stoi(argv[2]);
        Game game(diff);
        std::cout << "Автоигра на уровне " << diff << std::endl;
        int left = game.getLow(), right = game.getHigh();
        while (!game.isGameOver()) {
            int guess = (left + right) / 2;
            std::cout << "Компьютер пробует: " << guess << std::endl;
            std::string hint = game.checkGuess(guess);
            game.addAttempt(guess, hint);
            if (game.isWon()) {
                std::cout << "Угадал за " << game.getAttempts() << " попыток!" << std::endl;
                break;
            }
            if (hint.find("▲") != std::string::npos)
                left = guess + 1;
            else
                right = guess - 1;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        if (!game.isWon())
            std::cout << "Компьютер не справился :(" << std::endl;
    } else {
        playGame();
    }
    return 0;
}
