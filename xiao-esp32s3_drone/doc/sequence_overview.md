---
config:
  theme: redux
  fontFamily: '''Open Sans Variable'', sans-serif'
  themeVariables:
    fontFamily: '''Open Sans Variable'', sans-serif'
  look: neo
---
sequenceDiagram
  autonumber

  participant B as スマホ
  participant W
  participant S
  participant F

box マイコン
participant W as WEBサーバー
participant S as WEBソケット
participant F as フライトタスク
end

participant M as モーター


B->>W: 画面を開く
W-->>B: 操作画面を表示

B->>S: 操縦情報

S->>F: 操縦内容を渡す

F->>M: モーターの強さを決める

F-->>S: 現在の状態

S-->>B: 画面更新