// HotCold.java - Игра "Горячо-холодно" на Java
import java.util.*;
import java.io.*;
import java.nio.file.*;
import java.util.stream.Collectors;

class Game {
    private int difficulty;
    private int low, high, maxAttempts;
    private int secret;
    private int attempts;
    private List<HistoryEntry> history;
    private boolean won;
    private Integer record;
    private Random rand;

    private static class HistoryEntry {
        int guess;
        String hint;
        HistoryEntry(int g, String h) { guess = g; hint = h; }
    }

    public Game(int diff) {
        this.difficulty = diff;
        switch(diff) {
            case 1: low=1; high=50; maxAttempts=999; break;
            case 2: low=1; high=100; maxAttempts=10; break;
            case 3: low=1; high=200; maxAttempts=7; break;
            default: low=1; high=100; maxAttempts=10; break;
        }
        rand = new Random();
        secret = rand.nextInt(high - low + 1) + low;
        attempts = 0;
        history = new ArrayList<>();
        won = false;
        loadRecord();
    }

    public String checkGuess(int guess) {
        if (guess == secret) {
            won = true;
            return "ПОБЕДА!";
        }
        int dist = Math.abs(guess - secret);
        String hint = dist <= 10 ? "ГОРЯЧО" : dist <= 25 ? "ТЕПЛО" : "ХОЛОДНО";
        hint += (guess < secret) ? " ▲" : " ▼";
        return hint;
    }

    public void addAttempt(int guess, String hint) {
        attempts++;
        history.add(new HistoryEntry(guess, hint));
    }

    public boolean isGameOver() { return won || attempts >= maxAttempts; }

    public void saveRecord() {
        Map<String, Integer> records = loadAllRecords();
        String key = difficulty + "_" + low + "_" + high;
        if (!records.containsKey(key) || attempts < records.get(key)) {
            records.put(key, attempts);
            saveAllRecords(records);
        }
    }

    private Map<String, Integer> loadAllRecords() {
        Map<String, Integer> records = new HashMap<>();
        try {
            String content = new String(Files.readAllBytes(Paths.get("records.json")));
            // Очень простой парсинг (для демонстрации)
            content = content.replace("{", "").replace("}", "").trim();
            if (!content.isEmpty()) {
                String[] pairs = content.split(",");
                for (String pair : pairs) {
                    String[] kv = pair.split(":");
                    if (kv.length == 2) {
                        String key = kv[0].trim().replace("\"", "");
                        int val = Integer.parseInt(kv[1].trim());
                        records.put(key, val);
                    }
                }
            }
        } catch (IOException e) {}
        return records;
    }

    private void saveAllRecords(Map<String, Integer> records) {
        StringBuilder sb = new StringBuilder("{");
        boolean first = true;
        for (Map.Entry<String, Integer> e : records.entrySet()) {
            if (!first) sb.append(",");
            sb.append("\"").append(e.getKey()).append("\":").append(e.getValue());
            first = false;
        }
        sb.append("}");
        try {
            Files.write(Paths.get("records.json"), sb.toString().getBytes());
        } catch (IOException e) {}
    }

    private void loadRecord() {
        Map<String, Integer> records = loadAllRecords();
        String key = difficulty + "_" + low + "_" + high;
        record = records.get(key);
    }

    public void displayHistory() {
        System.out.println("\u001B[36m--- История попыток ---\u001B[0m");
        for (int i = 0; i < history.size(); i++) {
            HistoryEntry h = history.get(i);
            String color = h.hint.contains("ГОРЯЧО") ? "\u001B[31m" : "\u001B[34m";
            System.out.printf("%d: %d -> %s%s\u001B[0m%n", i+1, h.guess, color, h.hint);
        }
        System.out.println("------------------------");
    }

    public int getAttempts() { return attempts; }
    public int getSecret() { return secret; }
    public boolean hasRecord() { return record != null; }
    public int getRecord() { return record; }
    public int getLow() { return low; }
    public int getHigh() { return high; }
    public int getMaxAttempts() { return maxAttempts; }
    public boolean isWon() { return won; }
}

public class HotCold {
    private static Scanner scanner = new Scanner(System.in);

    public static void playGame() {
        System.out.println("\u001B[33m🔥❄️ Добро пожаловать в Hot&Cold!\u001B[0m");
        while (true) {
            System.out.println("\nВыберите уровень сложности:");
            System.out.println("1. Лёгкий (1-50, безлимит)");
            System.out.println("2. Средний (1-100, 10 попыток)");
            System.out.println("3. Сложный (1-200, 7 попыток)");
            System.out.println("4. Выйти");
            System.out.print("Ваш выбор: ");
            String choice = scanner.nextLine().trim();
            if (choice.equals("4")) {
                System.out.println("До свидания!");
                break;
            }
            if (!choice.matches("[1-3]")) {
                System.out.println("\u001B[31mНеверный выбор.\u001B[0m");
                continue;
            }
            int diff = Integer.parseInt(choice);
            Game game = new Game(diff);

            if (game.hasRecord()) {
                System.out.printf("\u001B[35mТекущий рекорд: %d попыток\u001B[0m%n", game.getRecord());
            } else {
                System.out.println("Рекорда пока нет.");
            }
            System.out.printf("\u001B[32mЧисло загадано! Диапазон %d-%d\u001B[0m%n", game.getLow(), game.getHigh());
            if (game.getMaxAttempts() < 999) {
                System.out.println("У вас " + game.getMaxAttempts() + " попыток.");
            } else {
                System.out.println("Попытки не ограничены.");
            }

            while (!game.isGameOver()) {
                System.out.printf("Попытка %d: Введите число: ", game.getAttempts()+1);
                String input = scanner.nextLine().trim();
                int guess;
                try {
                    guess = Integer.parseInt(input);
                } catch (NumberFormatException e) {
                    System.out.println("\u001B[31mВведите целое число!\u001B[0m");
                    continue;
                }
                if (guess < game.getLow() || guess > game.getHigh()) {
                    System.out.printf("\u001B[31mЧисло должно быть от %d до %d\u001B[0m%n", game.getLow(), game.getHigh());
                    continue;
                }
                String hint = game.checkGuess(guess);
                game.addAttempt(guess, hint);
                String color = hint.contains("ГОРЯЧО") ? "\u001B[31m" :
                               hint.contains("ХОЛОДНО") ? "\u001B[34m" : "\u001B[32m";
                System.out.println(color + hint + "\u001B[0m");

                if (game.isWon()) {
                    System.out.printf("\u001B[32m🎉 ПОБЕДА! Число %d угадано за %d попыток!\u001B[0m%n", game.getSecret(), game.getAttempts());
                    game.saveRecord();
                    if (game.hasRecord() && game.getAttempts() == game.getRecord()) {
                        System.out.println("\u001B[36m🏆 Новый рекорд!\u001B[0m");
                    }
                    break;
                } else if (game.getAttempts() >= game.getMaxAttempts()) {
                    System.out.printf("\u001B[31m😞 Вы исчерпали все попытки. Загаданное число: %d\u001B[0m%n", game.getSecret());
                    break;
                }
            }
            game.displayHistory();
            System.out.print("Хотите сыграть ещё раз? (y/n): ");
            String again = scanner.nextLine().trim().toLowerCase();
            if (!again.equals("y")) {
                System.out.println("До свидания!");
                break;
            }
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length > 0 && args[0].equals("--auto")) {
            int diff = args.length > 1 ? Integer.parseInt(args[1]) : 2;
            Game game = new Game(diff);
            System.out.println("Автоигра на уровне " + diff);
            int left = game.getLow(), right = game.getHigh();
            while (!game.isGameOver()) {
                int guess = (left + right) / 2;
                System.out.println("Компьютер пробует: " + guess);
                String hint = game.checkGuess(guess);
                game.addAttempt(guess, hint);
                if (game.isWon()) {
                    System.out.println("Угадал за " + game.getAttempts() + " попыток!");
                    break;
                }
                if (hint.contains("▲")) left = guess + 1;
                else right = guess - 1;
                Thread.sleep(500);
            }
            if (!game.isWon()) System.out.println("Компьютер не справился :(");
        } else {
            playGame();
        }
    }
}
