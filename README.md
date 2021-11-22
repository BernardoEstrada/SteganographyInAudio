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

Comando para insertar con mensaje:

```bash
./LSB.c -em  origin message destination
```

Comando para insertar con archivo:

```bash
./LSB -ef messagePath destination
```

Comando para extraer e imprimir:

```bash
./LSB -xp origin <Bits per byte>
```

Comando para extraer en un archivo:

```bash
./LSB -xf origin output <Bits per byte>
```

## Credits

Bernardo Estrada Fuentes

Francisco Agustín Díaz Vergara

Sebastian Resendiz
