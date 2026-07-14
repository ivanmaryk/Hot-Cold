// hotcold.cs - Игра "Горячо-холодно" на C#
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Threading;

class Game
{
    private int difficulty;
    private int low, high, maxAttempts;
    private int secret;
    private int attempts;
    private List<HistoryEntry> history;
    private bool won;
    private int? record;
    private Random rand = new Random();

    private struct HistoryEntry
    {
        public int Guess { get; set; }
        public string Hint { get; set; }
    }

    public Game(int diff)
    {
        difficulty = diff;
        switch(diff)
        {
            case 1: low=1; high=50; maxAttempts=999; break;
            case 2: low=1; high=100; maxAttempts=10; break;
            case 3: low=1; high=200; maxAttempts=7; break;
            default: low=1; high=100; maxAttempts=10; break;
        }
        secret = rand.Next(low, high+1);
        attempts = 0;
        history = new List<HistoryEntry>();
        won = false;
        LoadRecord();
    }

    public string CheckGuess(int guess)
    {
        if (guess == secret)
        {
            won = true;
            return "ПОБЕДА!";
        }
        int dist = Math.Abs(guess - secret);
        string hint = dist <= 10 ? "ГОРЯЧО" : dist <= 25 ? "ТЕПЛО" : "ХОЛОДНО";
        hint += guess < secret ? " ▲" : " ▼";
        return hint;
    }

    public void AddAttempt(int guess, string hint)
    {
        attempts++;
        history.Add(new HistoryEntry { Guess = guess, Hint = hint });
    }

    public bool IsGameOver() => won || attempts >= maxAttempts;

    public void SaveRecord()
    {
        var records = LoadAllRecords();
        string key = $"{difficulty}_{low}_{high}";
        if (!records.ContainsKey(key) || attempts < records[key])
        {
            records[key] = attempts;
            File.WriteAllText("records.json", JsonSerializer.Serialize(records));
        }
    }

    private Dictionary<string, int> LoadAllRecords()
    {
        if (!File.Exists("records.json")) return new Dictionary<string, int>();
        string json = File.ReadAllText("records.json");
        return JsonSerializer.Deserialize<Dictionary<string, int>>(json) ?? new Dictionary<string, int>();
    }

    private void LoadRecord()
    {
        var records = LoadAllRecords();
        string key = $"{difficulty}_{low}_{high}";
        if (records.ContainsKey(key)) record = records[key];
    }

    public void DisplayHistory()
    {
        Console.WriteLine("\u001b[36m--- История попыток ---\u001b[0m");
        for (int i = 0; i < history.Count; i++)
        {
            var h = history[i];
            string color = h.Hint.Contains("ГОРЯЧО") ? "\u001b[31m" : "\u001b[34m";
            Console.WriteLine($"{i+1}: {h.Guess} -> {color}{h.Hint}\u001b[0m");
        }
        Console.WriteLine("------------------------");
    }

    public int Attempts => attempts;
    public int Secret => secret;
    public bool HasRecord => record.HasValue;
    public int Record => record.Value;
    public int Low => low;
    public int High => high;
    public int MaxAttempts => maxAttempts;
    public bool Won => won;
}

class HotCold
{
    static void PlayGame()
    {
        Console.WriteLine("\u001b[33m🔥❄️ Добро пожаловать в Hot&Cold!\u001b[0m");
        while (true)
        {
            Console.WriteLine("\nВыберите уровень сложности:");
            Console.WriteLine("1. Лёгкий (1-50, безлимит)");
            Console.WriteLine("2. Средний (1-100, 10 попыток)");
            Console.WriteLine("3. Сложный (1-200, 7 попыток)");
            Console.WriteLine("4. Выйти");
            Console.Write("Ваш выбор: ");
            string choice = Console.ReadLine().Trim();
            if (choice == "4")
            {
                Console.WriteLine("До свидания!");
                break;
            }
            if (!int.TryParse(choice, out int diff) || diff < 1 || diff > 3)
            {
                Console.WriteLine("\u001b[31mНеверный выбор.\u001b[0m");
                continue;
            }
            var game = new Game(diff);

            if (game.HasRecord)
                Console.WriteLine($"\u001b[35mТекущий рекорд: {game.Record} попыток\u001b[0m");
            else
                Console.WriteLine("Рекорда пока нет.");

            Console.WriteLine($"\u001b[32mЧисло загадано! Диапазон {game.Low}-{game.High}\u001b[0m");
            if (game.MaxAttempts < 999)
                Console.WriteLine($"У вас {game.MaxAttempts} попыток.");
            else
                Console.WriteLine("Попытки не ограничены.");

            while (!game.IsGameOver())
            {
                Console.Write($"Попытка {game.Attempts+1}: Введите число: ");
                string input = Console.ReadLine().Trim();
                if (!int.TryParse(input, out int guess))
                {
                    Console.WriteLine("\u001b[31mВведите целое число!\u001b[0m");
                    continue;
                }
                if (guess < game.Low || guess > game.High)
                {
                    Console.WriteLine($"\u001b[31mЧисло должно быть от {game.Low} до {game.High}\u001b[0m");
                    continue;
                }
                string hint = game.CheckGuess(guess);
                game.AddAttempt(guess, hint);
                string color = hint.Contains("ГОРЯЧО") ? "\u001b[31m" :
                               hint.Contains("ХОЛОДНО") ? "\u001b[34m" : "\u001b[32m";
                Console.WriteLine($"{color}{hint}\u001b[0m");

                if (game.Won)
                {
                    Console.WriteLine($"\u001b[32m🎉 ПОБЕДА! Число {game.Secret} угадано за {game.Attempts} попыток!\u001b[0m");
                    game.SaveRecord();
                    if (game.HasRecord && game.Attempts == game.Record)
                        Console.WriteLine("\u001b[36m🏆 Новый рекорд!\u001b[0m");
                    break;
                }
                else if (game.Attempts >= game.MaxAttempts)
                {
                    Console.WriteLine($"\u001b[31m😞 Вы исчерпали все попытки. Загаданное число: {game.Secret}\u001b[0m");
                    break;
                }
            }
            game.DisplayHistory();
            Console.Write("Хотите сыграть ещё раз? (y/n): ");
            string again = Console.ReadLine().Trim().ToLower();
            if (again != "y")
            {
                Console.WriteLine("До свидания!");
                break;
            }
        }
    }

    static void Main(string[] args)
    {
        if (args.Length > 0 && args[0] == "--auto")
        {
            int diff = args.Length > 1 ? int.Parse(args[1]) : 2;
            var game = new Game(diff);
            Console.WriteLine($"Автоигра на уровне {diff}");
            int left = game.Low, right = game.High;
            while (!game.IsGameOver())
            {
                int guess = (left + right) / 2;
                Console.WriteLine($"Компьютер пробует: {guess}");
                string hint = game.CheckGuess(guess);
                game.AddAttempt(guess, hint);
                if (game.Won)
                {
                    Console.WriteLine($"Угадал за {game.Attempts} попыток!");
                    break;
                }
                if (hint.Contains("▲")) left = guess + 1;
                else right = guess - 1;
                Thread.Sleep(500);
            }
            if (!game.Won) Console.WriteLine("Компьютер не справился :(");
        }
        else
        {
            PlayGame();
        }
    }
}
