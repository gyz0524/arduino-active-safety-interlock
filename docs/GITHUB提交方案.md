# GitHub 提交方案

## 公开仓库建议

- 仓库名：`arduino-active-safety-interlock`
- 可见性：Public（如果只想自己保存，先选 Private）
- 默认分支：`main`
- 许可证：MIT
- 仓库根目录：直接放置 `111.ino`、`README.md`、`docs/`、`diagnostics/`、`src/` 和 `tests/`

不要把外层 Windows 文件夹名 `111` 再套一层上传。进入 `111` 文件夹后初始化 Git，GitHub 页面打开后直接能看到 `111.ino` 和 README。

## 发布前检查

1. 在 Arduino IDE 中选择 `Arduino Uno` 和正确串口。
2. 上传 `111.ino`，确认 LCD、TTP223B、摇杆、SW-520D、蜂鸣器和点阵按顺序工作。
3. 确认 `diagnostics/lcd1602_test/` 中的 LCD 测试程序仍能独立上传。
4. 在工程根目录运行 Python 状态机测试：

   ```powershell
   python -m unittest discover -s tests -v
   ```

5. 检查 Git 不会包含缓存、`.env`、IDE 临时文件或个人资料：

   ```powershell
   git status --short
   git check-ignore -v src/__pycache__ tests/__pycache__
   ```

6. 检查草图只使用 ASCII LCD 文案，避免 LCD1602 中文乱码。

## 首次提交

在 PowerShell 中执行：

```powershell
cd "C:\Users\Lenovo\Desktop\不知道\111"
git init
git branch -M main
git add .
git diff --cached --check
git status
git commit -m "feat: add Arduino active safety interlock console"
```

## 关联 GitHub 远程仓库并推送

先在 GitHub 新建一个空仓库，不要自动添加 README、许可证或 `.gitignore`，然后把下面的地址替换成自己的仓库地址：

```powershell
git remote add origin https://github.com/<你的用户名>/arduino-active-safety-interlock.git
git push -u origin main
```

如果使用 SSH：

```powershell
git remote add origin git@github.com:<你的用户名>/arduino-active-safety-interlock.git
git push -u origin main
```

## 后续修改提交

每次只提交一个明确主题：

```powershell
git status
git add 111.ino README.md docs diagnostics src tests
git diff --cached --check
git commit -m "fix: tune tilt input and buzzer feedback"
git push
```

推荐提交信息：

- `feat: add Arduino active safety interlock console`
- `docs: add wiring diagram and upload guide`
- `fix: improve LCD startup and buzzer feedback`
- `test: cover task state transitions`

## GitHub 页面检查

上传后确认：

- README 首页能看到项目功能、器件、接线和上传方法；
- `111.ino` 能直接在 Arduino IDE 打开；
- 清晰接线图可以从 `docs/接线图-清晰版.svg` 打开；
- LCD 独立测试在 `diagnostics/lcd1602_test/`；
- 没有 `__pycache__`、`.env`、个人路径、密码或串口截图；
- README 中没有声称已经完成未实际做过的硬件验证。

## 验证边界

Python 状态机测试可以验证待机、摇杆正确、倾斜正确、错误和超时逻辑。Arduino IDE 编译、串口选择、实际上传和传感器/蜂鸣器/点阵的物理表现仍需要在真实 Uno R3 上确认。
