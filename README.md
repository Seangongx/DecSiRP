# DecSiRP

**A Voting Scheme for Decomposing Surfaces into Round Primitives**

Source code accompanying the paper by Xun Gong, Tristan Roussillon, and Bertrand Kerautret (*submitted*).

## Dependencies

- CMake >= 3.6
- [DGtal](https://github.com/DGtal-team/DGtal) >= 2.1
- [Polyscope](https://github.com/nmwsharp/polyscope) >= 2.4.0 *(optional, for 3D visualization through DGtal's PolyscopeViewer)*


## Build
 ```bash
git clone https://github.com/Seangongx/DecSiRP.git
cd DecSiRP
cmake -S . -B build
cmake --build build
```
If DGtal is installed locally and CMake cannot find it automatically, specify its build directory:

```bash
cmake -S . -B build -DDGtal_DIR=/path/to/DGtal/build
cmake --build build
```

## Typical use

```bash
./build/viz -m ../Samples/hand.obj
```


## Citation

If you use this code in your research, please cite the associated paper once it is published.
