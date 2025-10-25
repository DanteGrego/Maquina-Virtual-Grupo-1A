# Trabajo Práctico: Máquina Virtual — Parte II  
### Arquitectura de Computadoras – MV2 2025  
**Universidad Nacional de Mar del Plata — Facultad de Ingeniería**

---

## Descripción

Este proyecto implementa la segunda parte de la Máquina Virtual (VMX) solicitada en la materia *Arquitectura de Computadoras*.  
La aplicación permite ejecutar programas en lenguaje máquina (`.vmx`), continuar ejecuciones desde imágenes (`.vmi`) y mostrar su estado en modo desensamblado (`-d`).  

Además, se agregan nuevas funcionalidades:  
- Soporte de hasta seis segmentos de memoria (Param, Const, Code, Data, Extra, Stack).  
- Manejo de pila con instrucciones `PUSH`, `POP`, `CALL` y `RET`.  
- Llamadas al sistema para entrada/salida de texto, limpieza de pantalla y breakpoints.  
- Generación y lectura de imágenes de máquina virtual (`.vmi`).


PD: La llamamos El Bicho 👾

---

## Prerrequisitos

Antes de compilar o ejecutar el proyecto, se requiere:

- **Sistema operativo:** Windows 10/11 (probado en entorno MSYS2 / UCRT64)
- **Compilador:** `gcc` (instalado con MSYS2 o MinGW)
- **Editor recomendado:** Visual Studio Code (configuración incluida)
- **Extensión C/C++** para VSCode (para detectar `tasks.json`)

---

## Intalacion y configuracion

1. Descargar el proyecto:
```bash
   git clone https://github.com/DanteGrego/Maquina-Virtual-Grupo-1A
   cd tp-mv2-2025
 ```

2. Abrir en Visual Studio Code:

- Archivo → Abrir carpeta → seleccionar el proyecto.

- Confirmar que existe la carpeta `.vscode` con `tasks.json.`

3. Compilar la máquina virtual:

- Presionar `Ctrl + Shift + B` y elegir la tarea “Compilar vmx.exe”,
o ejecutar manualmente:
```bash
cd source
gcc -g vmx.c operaciones.c dissasembler.c consts.c gettersetter.c sys.c inicializacion.c -o ../debug/vmx.exe
```

---

## Ejecucion 

La máquina virtual puede ejecutarse desde consola o mediante las tareas configuradas en VSCode.  
El formato general del comando es:

```bash
vmx [programa.vmx] [imagen.vmi] [m=M] [-d] [-p param1 param2 ...]
```

### Modos de ejecucion

La máquina virtual admite tres formas de uso
1. **Ejecución desde un programa (`.vmx`)**

Crea un nuevo proceso en memoria y comienza su ejecución desde el entry point definido en el archivo.
```bash
vmx test.vmx -p hola mundo
```

2. **Reanudación desde una imagen (`.vmi`)**

Restaura un estado previo de la máquina virtual y continúa la ejecución exactamente desde el punto en que se generó la imagen.
```bash
vmx test.vmi
```
3. **Ejecución combinada (`.vmx` + `.vmi`)**

Permite ejecutar un nuevo programa y generar automáticamente una imagen .vmi cada vez que se alcance un breakpoint (SYS 0xF).
```bash
vmx test.vmx test.vmi -d
```

**Modo desensamblado (-d)**

Si se incluye el flag -d, la VM muestra antes de ejecutar:

- El contenido del Const Segment (KS) con las cadenas constantes 
- El código assembler desensamblado, indicando el entry point con >.

- Los operandos con sus pseudónimos de registro y modificadores (`b[]`, `w[]`, `l[]`).


**Ejemplo:**
```bash
>[002A] B1 00 0A 9B 00 05  |  ADD w[DS+5], 10
 [0030] 96 00 08 CA        |  SHL AX, 8
 [0034] 48 8E              |  NOT EH
```


**Ejecución con debugger visual**

La cátedra provee el ejecutable `vmg.exe` para observar el estado interno de la máquina en tiempo real.
Este programa lee el archivo .vmi y muestra los registros, segmentos y código desensamblado.
```bash
vmg test.vmi -r -s w=5
```

**Parámetros del debugger:**

| Opción |	Descripción |
|:---------|:------------|
| -r	| Muestra los valores de los registros.|
| -s	| Muestra la tabla de segmentos.|
| w=W	| Define la cantidad de líneas de código mostradas alrededor del IP.|

**Resumen de combinaciones válidas**

| Comando |	Acción |
|:---------|:------------|
| vmx programa.vmx	| Ejecuta un nuevo programa. |
| vmx programa.vmx -p ... |	Ejecuta con parámetros. |
| vmx imagen.vmi |	Retoma una ejecución previa. |
| vmx programa.vmx imagen.vmi |	Ejecuta y genera imagen en breakpoints.|
|vmx programa.vmx -d |	Muestra el código assembler antes de ejecutar.|


---

## Integrantes

- [Santiago Martinez](https://github.com/sneakygrinder)
- [Dante Emilio Gregorini](https://github.com/DanteGrego)
- [Ianai Gribman](https://github.com/IanaiGribman)

## Docentes

**Cátedra de Arquitectura de Computadoras**

Facultad de Ingeniería – Universidad Nacional de Mar del Plata

- Ing. Pablo A. Montini
- Ing. Juan I. Iturriaga
- Ing. Franco Lanzillota