# Steganography in audio

This project is for the class "Seguridad informática" for a Steganography in audio project made in C.

## Installation

```bash
git clone https://github.com/BernardoEstrada/SteganographyInAudio
```

## Usage

Compilar

```bash
gcc -o [executable name] LSB.c
```

Nombre del ejecutable en nuestro caso: LSB

Comando para imprimir mensaje de ayuda:

```bash
./LSB -h
```

Comando para encodear con mensaje:

```bash
./LSB.c -em  origin message destination
```

Comando para encodear con archivo:

```bash
./LSB -ef messagePath destination
```

Comando para decodificar e imprimir:

```bash
./LSB -dp origin <Bits per byte>
```

Comando para decodificar en un archivo:

```bash
./LSB -df origin output <Bits per byte>
```

## Credits

Bernardo Estrada Fuentes

Francisco Agustín Díaz Vergara

Sebastian Resendiz
