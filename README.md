# Lab 009 – Virtual Address Translation and Page Tables

Simulacion de traduccion de direcciones virtuales a fisicas usando una page table lineal,
un allocator de frames fisicos y una RAM de 100 frames.

## Compilacion

```bash
gcc -o lab009 main.c ram_allocator.c process.c translator.c
```

## Uso

```bash
./lab009 <num_virtual_pages> <address_file> [seed]
```

- `num_virtual_pages` — cuantas paginas virtuales tiene el proceso (V), entre 1 y 100.
- `address_file` — archivo de texto con una direccion virtual por linea (decimal o 0x hex).
- `seed` — opcional, semilla para el generador aleatorio. Si no se pasa usa `time(NULL)`.

Ejemplo:
```bash
./lab009 8 example_addresses.txt
./lab009 8 example_addresses.txt 12345
```

## Estructura de archivos

```
ram_allocator.h / ram_allocator.c   →  RAM fisica y allocator de frames
process.h       / process.c         →  struct del proceso y page table
translator.h    / translator.c      →  traduccion de direcciones virtuales
main.c                              →  punto de entrada, orquesta todo
visualize.py                        →  visualizacion 2D a color (punto extra)
example_addresses.txt               →  dataset de referencia del lab
```

---

## ram_allocator.h / ram_allocator.c

Maneja la RAM fisica: 100 frames, cada uno con estado FREE u OCCUPIED y un owner.

**Constantes**
- `NUM_FRAMES 100` — total de frames fisicos.
- `FRAME_FREE / FRAME_OCCUPIED` — estados posibles de un frame.
- `OWNER_NONE / OWNER_PROC1 / OWNER_PROC2` — a quien pertenece el frame.

**Structs**
- `Frame` — un frame fisico: tiene `state` y `owner`.
- `RAM` — arreglo de 100 frames, contadores `free_count` y `occupied_count`, y la `seed` usada.

**Funciones**

`void ram_init(RAM *ram)`
Inicializa todos los frames como FREE y pone los contadores en 0. Se llama antes de randomizar.

`unsigned int ram_randomize(RAM *ram, unsigned int seed, int need)`
Marca entre 10 y 60 frames aleatorios como OCCUPIED para simular memoria ya en uso.
Si `free_count` resultante es menor que `max(10, need)` reintenta con la siguiente seed
(hasta 50 veces). Retorna la seed que se uso.

`int ram_allocate_frame(RAM *ram, int owner)`
Busca el primer frame FREE, lo marca OCCUPIED con el owner dado y retorna su indice.
Retorna -1 si no hay frames libres.

`void ram_free_frame(RAM *ram, int frame_idx)`
Libera un frame (lo vuelve FREE). Se usa solo en el rollback del load.

`void ram_print_map(const RAM *ram)`
Imprime el mapa de los 100 frames en 10 columnas. Cada frame muestra su indice y estado:
F = libre, X = pre-existente, 1 = Proceso 1, 2 = Proceso 2.

---

## process.h / process.c

Define el proceso y su page table lineal.

**Structs**

`PTE` — Page Table Entry, una entrada de la page table. Tiene:
- `valid` (bool) — si esta pagina tiene un frame asignado.
- `pfn` (uint8_t) — que frame fisico le toco (0..99).

`Process` — representa un proceso. Tiene:
- `pid` — identificador del proceso.
- `num_virtual_pages` — V, cuantas paginas virtuales usa.
- `page_table[256]` — arreglo de PTEs, se indexa por VPN.
- `owner_id` — label para saber que frames en RAM le pertenecen.

**Funciones**

`void process_init(Process *proc, int pid, int num_pages, int owner_id)`
Inicializa el proceso: pone todos los PTEs en `valid = false`. Se llama antes del load.

`bool process_load(Process *proc, RAM *ram)`
Recorre VPN 0 hasta V-1 y le asigna un frame fisico a cada uno llamando a `ram_allocate_frame`.
Si falla a mitad hace rollback: libera todos los frames ya asignados y pone sus PTEs en `valid = false`.
En la practica no falla porque `ram_randomize` garantiza `free_count >= V` antes del load.

`void process_print_page_table(const Process *proc)`
Imprime la page table: VPN, si es valida, y el PFN asignado.

---

## translator.h / translator.c

Toda la logica de traduccion de direcciones virtuales a fisicas.

**Constantes**
- `PAGE_SIZE 256` — tamano de pagina y frame en bytes.
- `VA_MAX 0xFFFF` — direccion virtual maxima valida (16 bits).

**Structs**

`TransResult` — enum con los posibles resultados:
- `TRANS_OK` — traduccion exitosa.
- `TRANS_VA_OUT_OF_RANGE` — la direccion supera 0xFFFF.
- `TRANS_VPN_OUT_OF_RANGE` — el VPN es mayor o igual a V.
- `TRANS_PAGE_NOT_MAPPED` — el PTE tiene `valid = false`.

`Translation` — resultado completo de una traduccion: VA, VPN, offset, PFN, PA y el codigo de resultado.

**Funciones**

`bool va_decompose(uint32_t va, uint8_t *vpn, uint8_t *offset)`
Descompone una direccion virtual en VPN y offset:
- `offset = va & 0xFF` (bits 7:0)
- `vpn    = (va >> 8) & 0xFF` (bits 15:8)
Retorna false si va > 0xFFFF.

`Translation translate(uint32_t va, const Process *proc)`
Traduce una direccion virtual aplicando tres validaciones en orden:
1. Si va > 0xFFFF → `VA_OUT_OF_RANGE`.
2. Si vpn >= V    → `VPN_OUT_OF_RANGE`.
3. Si PTE invalido → `PAGE_NOT_MAPPED`.
Si pasa todo: `PA = PFN * 256 + offset`.

`void translation_print(const Translation *t)`
Imprime una linea con el resultado de la traduccion.

`int translate_file(const char *file_path, const Process *proc)`
Lee un archivo linea por linea, parsea cada direccion (decimal o 0x hex),
llama a `translate` y imprime el resultado. Retorna cuantas direcciones proceso, -1 si no pudo abrir el archivo.

---

## main.c

Orquesta los 4 pasos del lab en orden:

1. Parsea argumentos del CLI.
2. Llama a `ram_randomize` e imprime el mapa de RAM.
3. Llama a `process_load` e imprime la page table.
4. Llama a `translate_file` para traducir todas las direcciones.

Si se pasa `--proc2` repite los pasos 3 y 4 para un segundo proceso sobre la misma RAM.

---

## Punto extra: segundo proceso

Con `--proc2` se carga un segundo proceso sobre el mismo pool de RAM. Los frames del
proceso 1 siguen OCCUPIED, el proceso 2 solo puede usar los que queden libres.
En el mapa de consola los frames del proceso 1 se muestran como `1` y los del proceso 2 como `2`.

```bash
./lab009 8 example_addresses.txt 12345 --proc2 4 example_addresses.txt
```

---

## Punto extra: visualizacion 2D a color

`visualize.py` lee la salida del programa y genera `ram_visual.html` con una grilla
10x10 de los 100 frames, cada uno con color segun su estado:

- Gris — frame libre
- Rojo — pre-existente
- Azul — frame del Proceso 1
- Verde — frame del Proceso 2

```bash
./lab009 8 example_addresses.txt 12345 | python3 visualize.py
```

El HTML generado muestra el estado de la RAM despues del load. No es interactivo
porque la RAM no cambia durante la ejecucion (no hay stores reales, solo traduccion de direcciones).
