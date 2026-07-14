// hotcold.js - Игра "Горячо-холодно" на JavaScript (Node.js)
const readline = require('readline').createInterface({
    input: process.stdin,
    output: process.stdout
});
const fs = require('fs').promises;

class Game {
    constructor(difficulty) {
        this.difficulty = difficulty;
        const ranges = {
            1: { low: 1, high: 50, maxAttempts: Infinity },
            2: { low: 1, high: 100, maxAttempts: 10 },
            3: { low: 1, high: 200, maxAttempts: 7 }
        };
        const { low, high, maxAttempts } = ranges[difficulty] || ranges[2];
        this.low = low;
        this.high = high;
        this.maxAttempts = maxAttempts;
        this.secret = Math.floor(Math.random() * (high - low + 1)) + low;
        this.attempts = 0;
        this.history = [];
        this.won = false;
        this.record = null;
        this.loadRecord();
    }

    checkGuess(guess) {
        if (guess === this.secret) {
            this.won = true;
            return "ПОБЕДА!";
        }
        const dist = Math.abs(guess - this.secret);
        let hint = dist <= 10 ? "ГОРЯЧО" : dist <= 25 ? "ТЕПЛО" : "ХОЛОДНО";
        hint += guess < this.secret ? " ▲" : " ▼";
        return hint;
    }

    addAttempt(guess, hint) {
        this.attempts++;
        this.history.push({ guess, hint });
    }

    isGameOver() {
        return this.won || this.attempts >= this.maxAttempts;
    }

    async saveRecord() {
        let records = {};
        try {
            const data = await fs.readFile('records.json', 'utf8');
            records = JSON.parse(data);
        } catch {}
        const key = `${this.difficulty}_${this.low}_${this.high}`;
        if (!records[key] || this.attempts < records[key]) {
            records[key] = this.attempts;
            await fs.writeFile('records.json', JSON.stringify(records, null, 2));
            return true;
        }
        return false;
    }

    async loadRecord() {
        try {
            const data = await fs.readFile('records.json', 'utf8');
            const records = JSON.parse(data);
            const key = `${this.difficulty}_${this.low}_${this.high}`;
            this.record = records[key] || null;
        } catch {}
    }

    displayHistory() {
        console.log('\x1b[36m--- История попыток ---\x1b[0m');
        this.history.forEach((h, i) => {
            const color = h.hint.includes('ГОРЯЧО') ? '\x1b[31m' : '\x1b[34m';
            console.log(`${i+1}: ${h.guess} -> ${color}${h.hint}\x1b[0m`);
        });
        console.log('------------------------');
    }
}

function askQuestion(query) {
    return new Promise(resolve => readline.question(query, resolve));
}

async function playGame() {
    console.log('\x1b[33m🔥❄️ Добро пожаловать в Hot&Cold!\x1b[0m');
    while (true) {
        console.log('\nВыберите уровень сложности:');
        console.log('1. Лёгкий (1-50, безлимит)');
        console.log('2. Средний (1-100, 10 попыток)');
        console.log('3. Сложный (1-200, 7 попыток)');
        console.log('4. Выйти');
        const choice = await askQuestion('Ваш выбор: ');
        if (choice === '4') {
            console.log('До свидания!');
            readline.close();
            process.exit(0);
        }
        if (!['1','2','3'].includes(choice)) {
            console.log('\x1b[31mНеверный выбор.\x1b[0m');
            continue;
        }
        const diff = parseInt(choice);
        const game = new Game(diff);
        await game.loadRecord();

        if (game.record !== null) {
            console.log(`\x1b[35mТекущий рекорд: ${game.record} попыток\x1b[0m`);
        } else {
            console.log('Рекорда пока нет.');
        }
        console.log(`\x1b[32mЧисло загадано! Диапазон ${game.low}-${game.high}\x1b[0m`);
        if (game.maxAttempts < Infinity) {
            console.log(`У вас ${game.maxAttempts} попыток.`);
        } else {
            console.log('Попытки не ограничены.');
        }

        while (!game.isGameOver()) {
            const input = await askQuestion(`Попытка ${game.attempts+1}: Введите число: `);
            const guess = parseInt(input);
            if (isNaN(guess)) {
                console.log('\x1b[31mВведите целое число!\x1b[0m');
                continue;
            }
            if (guess < game.low || guess > game.high) {
                console.log(`\x1b[31mЧисло должно быть от ${game.low} до ${game.high}\x1b[0m`);
                continue;
            }
            const hint = game.checkGuess(guess);
            game.addAttempt(guess, hint);
            let color = '\x1b[31m';
            if (hint.includes('ХОЛОДНО')) color = '\x1b[34m';
            else if (hint.includes('ПОБЕДА')) color = '\x1b[32m';
            console.log(`${color}${hint}\x1b[0m`);

            if (game.won) {
                console.log(`\x1b[32m🎉 ПОБЕДА! Число ${game.secret} угадано за ${game.attempts} попыток!\x1b[0m`);
                const newRecord = await game.saveRecord();
                if (newRecord) console.log('\x1b[36m🏆 Новый рекорд!\x1b[0m');
                break;
            } else if (game.attempts >= game.maxAttempts) {
                console.log(`\x1b[31m😞 Вы исчерпали все попытки. Загаданное число: ${game.secret}\x1b[0m`);
                break;
            }
        }
        game.displayHistory();
        const again = await askQuestion('Хотите сыграть ещё раз? (y/n): ');
        if (again.toLowerCase() !== 'y') {
            console.log('До свидания!');
            readline.close();
            process.exit(0);
        }
    }
}

// Аргументы командной строки
if (process.argv.includes('--auto')) {
    const idx = process.argv.indexOf('--auto');
    const diff = parseInt(process.argv[idx+1]) || 2;
    (async () => {
        const game = new Game(diff);
        await game.loadRecord();
        console.log(`Автоигра на уровне ${diff}`);
        let left = game.low, right = game.high;
        while (!game.isGameOver()) {
            const guess = Math.floor((left + right) / 2);
            console.log(`Компьютер пробует: ${guess}`);
            const hint = game.checkGuess(guess);
            game.addAttempt(guess, hint);
            if (game.won) {
                console.log(`Угадал за ${game.attempts} попыток!`);
                break;
            }
            if (hint.includes('▲')) left = guess + 1;
            else right = guess - 1;
            await new Promise(resolve => setTimeout(resolve, 500));
        }
        if (!game.won) console.log('Компьютер не справился :(');
        readline.close();
    })();
} else {
    playGame();
}
