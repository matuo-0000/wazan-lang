# 和算（Wazan）プログラミング言語

古文調の日本語で書けるインタープリタ型プログラミング言語です。

## 特徴

- 古文調の日本語構文
- インタープリタ型実行
- 変数、関数、演算、条件分岐、ループをサポート
- C 言語で実装された高速な実行環境
- 字句解析器、構文解析器、インタープリタの 3 層アーキテクチャ

## ビルドとインストール

```bash
cd wazan
make
```

`make`コマンドを実行すると、自動的に`/usr/local/bin/wazan`にインストールされます。
（sudo パスワードの入力が必要な場合があります）

### ビルドのみ（インストールしない）

```bash
cd wazan
make build-only
```

### 手動インストール

```bash
cd wazan
make install          # /usr/local/bin にインストール
make install-user     # ~/.local/bin にインストール（sudoなし）
```

## 実行方法

### インストール後

```bash
wazan sample.wz
wazan examples/hello.wz
```

### 対話モード

```bash
wazan
```

### ビルドディレクトリから直接実行

```bash
cd wazan
./wazan_interp ../sample.wz
```

### テスト実行

```bash
cd wazan
make test
```

## 言語仕様

### 変数宣言と演算

```wz
数 x は 二 と 三 の和 と定む。
数 y は 九 と 三 の差 と定む。
数 z は 四 と 五 の積 と定む。
数 w は 八 と 二 の商 と定む。
```

### 出力

```wz
申す x
申す「こんにちは」
```

### 関数定義と呼び出し

```wz
術「和」(数 a、数 b) は a と b の和 と定む。
数 result は 和(二、三)
申す result
```

### 条件分岐

```wz
数 x は 三 と定む。
もし x 二より大なれば 申す「大なり」 さらずば 申す「小なり」
```

### ループ

```wz
いざ i を 一とし、五未満なる間、ひとつずつ加え行かん。
```

## サンプルコード

### examples/hello.wz

```wz
申す「こんにちは、和算の世界へ」
```

### examples/calc.wz

```wz
申す「四則演算のデモ」
数 a は 二 と 三 の和 と定む。
申す a
数 b は 九 と 三 の差 と定む。
申す b
数 c は 四 と 五 の積 と定む。
申す c
数 d は 八 と 二 の商 と定む。
申す d
```

### examples/function.wz

```wz
申す「関数のテスト」
術「和」(数 a、数 b) は a と b の和 と定む。
数 z は 和(二、三)
申す z
術「積」(数 x、数 y) は x と y の積 と定む。
数 w は 積(四、五)
申す w
```

## プロジェクト構造

```
.
├── include/          # ヘッダーファイル
│   ├── lexer.h      # 字句解析器
│   ├── parser.h     # 構文解析器
│   └── interpreter.h # インタープリタ
├── src/             # ソースファイル
│   ├── lexer.c      # 字句解析実装
│   ├── parser.c     # 構文解析実装
│   ├── interpreter.c # インタープリタ実装
│   └── main.c       # メインプログラム
├── Makefile         # ビルド設定
└── sample.wz        # サンプルコード
```

## アーキテクチャ

1. **字句解析器（Lexer）**: ソースコードをトークンに分割
2. **構文解析器（Parser）**: トークンから AST（抽象構文木）を生成
3. **インタープリタ（Interpreter）**: AST を実行

詳細は[ARCHITECTURE.md](ARCHITECTURE.md)を参照してください。

## アンインストール

```bash
cd wazan
make uninstall        # /usr/local/bin から削除
make uninstall-user   # ~/.local/bin から削除
```

## トラブルシューティング

### sudo パスワードを求められる

`make`実行時に`/usr/local/bin`への書き込み権限がない場合、sudo パスワードの入力が必要です。
sudo を使いたくない場合は、`make install-user`を使用してください。

### コマンドが見つからない

`~/.local/bin`にインストールした場合、PATH に追加する必要があります：

```bash
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

## ライセンス

MIT License
