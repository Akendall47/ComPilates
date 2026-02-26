# Assembly Spacing Convention

Operands always start at column 8.

Formula: `mnemonic + (8 - len(mnemonic)) spaces`

eg

| Mnemonic | Length | Spaces added | Total |
|----------|--------|--------------|-------|
| `j`      | 1      | 7            | 8     |
| `mv`     | 2      | 6            | 8     |
| `ret`    | 3      | 5            | 8     |
| `addi`   | 4      | 4            | 8     |

Labels start at column 0 (no leading spaces):
```
.Lfor_0:       <- column 0, no indent
addi ...       <- column 8 (mnemonic rule)
```
