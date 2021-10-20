# Python evolutionary embedding

## How to use?
From within the python folder, execute
(exact command will probably change frequently in the near future):

```
python -m src.evolution
```

## Inputs
- `H`: Minor to embed
- `G`: Graph, e.g. a Chimera or Pegasus graph

## Algorithm
- First step: init basic path with length `n`
<br>(`n` is the number of nodes in the minor H)

![Init basic path](./images/init_path.svg)

- More to come