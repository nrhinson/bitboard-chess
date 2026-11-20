# Bitboard Chess

This is a demo version of chess, meant to display the functionalities of bitwise operators!

I took on this project to help me attempt to better understand bitwise operators and their applications.
These mainly included learning how to bitshift to move pieces, and use operators such as AND, OR, XOR and NOT to control the logic of the game.
I learned how to prevent looping, and also learned how certain pieces use iterative logic. 

For more information, check out the presentation posted in the repo!


# The following are instructions for the rules, and how to install the demo.

**Limitations:** This is a demo version. Castling, en passant, and pawn promotion
are not supported, and the game does not stop you from moving into or staying in
check. The game ends when a king is captured.

---

## Windows (RECOMENDED): Run the EXE

1. Go to the **Releases** page of this repository and download `chess.exe`.
2. Double-click `chess.exe` to run it.
   - If Windows shows **"Windows protected your PC"**, click **More info → Run anyway**.
     This warning appears because the file isn't digitally signed.
3. Alternatively, open Command Prompt or PowerShell in the download folder and run:
   ```
   .\chess.exe
   ```

---

## Linux: Compile with GCC

### 1. Install GCC

Debian / Ubuntu:
```bash
sudo apt update
sudo apt install build-essential
```

Fedora:
```bash
sudo dnf install gcc
```

Arch:
```bash
sudo pacman -S gcc
```

This project uses C23 features, so you need **GCC 13 or newer**. Check your version with:
```bash
gcc --version
```

### 2. Download the source

```bash
git clone https://github.com/YOUR-USERNAME/YOUR-REPO.git
cd YOUR-REPO
```

### 3. Compile

```bash
gcc -std=c2x -o chess Chessboard.c
```

### 4. Run

```bash
./chess
```

---

## How to Play

- White moves first; players alternate turns.
- When prompted, enter a square as a file letter and rank number, e.g. `e2`.
  - **"Move the piece at:"** — the square of the piece you want to move.
  - **"To the square at:"** — where you want it to go.
- Answer `Y` or `N` (capital letters) when asked to start or play again.
- Type `00` at any square prompt to quit.# bitboard-chess