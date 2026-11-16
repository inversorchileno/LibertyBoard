# LibertyBoard

LibertyBoard es un fork de OpenBoard con un puntero láser estilo Drawboard PDF y una interfaz más tradicional, en ventana. Este archivo reúne lo acordado con el dueño del proyecto, para que cualquier sesión de trabajo nueva pueda retomarlo.

## Etapas

1. **Compilación para Windows, láser nuevo y orden de las barras.** *(hecha)*
2. **Paneles de opciones estilo Drawboard y formas nuevas.** *(hecha)*
3. **Rediseño a ventana normal con menús (modo pizarra y modo documentos).** *(hecha)*

## 1. Barras de herramientas

Ambas barras van en gris oscuro sólido, sin transparencia.

**Modo escritorio**, en orden: Modo pizarra, Lápiz, Borrador, Rotulador, Láser, Selección, Flecha, Rectángulo, Elipse, Subrayar, Texto. Se quitan el teclado virtual, la captura de área y la captura de pantalla.

**Modo pizarra**, en orden: Lápiz, Borrador, Rotulador, Láser, Selección, Interactuar, Mano, Zoom +, Zoom −, Línea, Texto, Flecha, Rectángulo, Elipse, Subrayar. Se quitan la captura, el teclado virtual y el imán de la cuadrícula.

## 2. Opciones de cada herramienta (estilo Drawboard)

En modo escritorio son paneles emergentes; en modo pizarra, un panel fijo a la derecha. Los paneles son fijos: sin candado, papelera ni "…".

- **Lápiz:** tipo de trazo Básico (ancho fijo), Pro (ancho variable) o Caligrafía (ancho según dirección); grosor 1–50; 9 colores más un selector de color; opacidad.
- **Goma:** Whole (borra el trazo completo) o Partial (borra por donde pasa); tamaño 1–50.
- **Rotulador:** 9 colores de resaltador más selector; grosor 20–100; opacidad.
- **Rectángulo y elipse:** borde y relleno con colores independientes; grosor; tipo de línea (continua, punteada, guiones…); borde recto u ondulado tipo nube; opacidad.
- **Flecha:** colores; grosor; tipo de línea; puntas de inicio y fin configurables (ninguna, cuadrado, círculo, rombo, flecha abierta, flecha cerrada…); opacidad.
- **Subrayar:** línea recta horizontal, 100 % opaca, trazada bajo la palabra; colores y grosor.
- **Texto:** fuente, tamaño, color, negrita y cursiva.
- **Láser:** modo Trail (estela) o Point (punto); grosor 0,5–64; colores; efecto Standard o White burn (centro blanco con halo); opacidad; tiempo de espera y duración del desvanecimiento de 100 % a 0 %.
- Los últimos ajustes de cada herramienta se recuerdan al volver a abrir el programa.

## 3. Modo pizarra en ventana normal

- Ventana normal (maximizable), con la barra de tareas de Windows visible.
- Barra de herramientas a la izquierda, panel de opciones fijo a la derecha.
- Menú superior tradicional con Pizarra, Web, Documentos, Escritorio y Configuración.
- Fila de íconos bajo el menú: Deshacer y Repetir; Página nueva, Anterior y Siguiente; Fondos y Borrar.
- Sin las pestañas laterales de miniaturas ni de biblioteca.
- En modo escritorio desaparece todo y queda solo la barra flotante.
- El modo Web recibe el mismo tratamiento: ventana normal y menú superior.

## 4. Modo Documentos en ventana normal

- Mismo menú superior que la pizarra.
- Menú nuevo "Documento" con todas las acciones de la barra inferior izquierda: Nuevo documento, Nueva carpeta, Importar, Exportar, Cambiar nombre, Favorito, Duplicar, Papelera, Abrir en la pizarra, Añadir al documento y Añadir…, con atajos de teclado e íconos.
- Fila de íconos con las acciones frecuentes: Nuevo documento, Importar, Exportar, Abrir en la pizarra y Papelera.
- Se elimina el teclado virtual flotante en todo el programa.

## Cómo quedó la Etapa 2

- Las opciones de cada herramienta se abren en una ventanita estilo Drawboard:
  - en modo escritorio, con la flechita del botón o manteniendo presionado el botón;
  - en modo pizarra, con doble clic sobre la herramienta. En la Etapa 3 pasan al panel fijo de la derecha.
- Los ajustes se guardan al cerrar el programa.
- La flecha, el rectángulo, la elipse y el subrayado se guardan como trazos (polígonos), igual que el lápiz: se pueden mover, borrar y deshacer, y cualquier versión de OpenBoard abre los documentos. Una vez dibujados no se editan como forma (no se cambia su relleno después).
- Mantener Shift dibuja cuadrados y círculos, y flechas en ángulos de 15 grados.
- Los trazos Pro y Caligrafía dependen del lápiz: con mouse el Pro se ve como el Básico.
- El color del lápiz y del rotulador reemplaza el color seleccionado de la barra inferior de la pizarra, y el grosor reemplaza el grosor seleccionado (fino, medio o grueso).

## Cómo quedó la Etapa 3

- El programa abre en una ventana maximizada, con la barra de tareas de Windows visible. El modo escritorio sigue cubriendo toda la pantalla.
- Menú superior: **Ver** (Pizarra, Web, Documentos, Mostrar escritorio), **Documento** (solo en el modo Documentos) y **LibertyBoard** (tamaño de página, copiar y pegar, tutorial, preferencias, podcast, salir).
- Modo pizarra: barra de herramientas a la izquierda, fila de íconos bajo el menú (Deshacer y Repetir; Páginas, Anterior y Siguiente; Fondos y Borrar) y panel de opciones fijo a la derecha, que cambia con la herramienta elegida. El doble clic sobre una herramienta sigue abriendo sus opciones junto al botón.
- Modo Documentos: fila con Nuevo documento, Importar, Exportar, Abrir en la pizarra y Papelera; el resto en el menú Documento, con atajos: Ctrl+N, Ctrl+Mayús+N, Ctrl+I, F2, Ctrl+Mayús+D, Supr y Ctrl+O.
- Sin pestañas laterales (miniaturas y biblioteca) ni teclado virtual.
- La ventana y la barra vertical vienen así por defecto en la configuración de LibertyBoard (`RunInWindow=true`, `ToolBarOrientationVertical=true`).

## Ajustes después de la Etapa 3

- Abre en modo escritorio, con la barra desplegada.
- La barra de herramientas de la pizarra va por defecto a la derecha. Al soltarla junto al borde de arriba o de abajo se vuelve horizontal; junto a la izquierda o la derecha, vertical. LibertyBoard recuerda el borde.
- La fila de íconos está arriba (con su propio ajuste, independiente de OpenBoard) y muestra las opciones de la herramienta elegida: ya no hay panel a la derecha. Páginas, Anterior, el contador de páginas, Siguiente, Fondos y Borrar quedan a la derecha.
- En el menú, a la derecha: botón **Escritorio** y un triángulo que oculta o muestra la fila de íconos.
- Contador de páginas «1 / 200»: se escribe un número y Enter lleva a esa página.
- Barra lateral izquierda con las miniaturas de las páginas, que se oculta o muestra con un triángulo.
- Al importar un archivo (por ejemplo un PDF) se abre de inmediato en la pizarra, en la hoja 1.
- **Resaltar:** una banda horizontal semitransparente que se arrastra sobre una línea de texto, con colores, alto y opacidad. No detecta el texto del PDF: se arrastra a lo largo de la línea.
- **Nota:** un clic pone un marcador con un texto. Al pasar el puntero se ve el texto; al hacer clic (con la herramienta Nota, Selección o Interactuar) se puede actualizar o borrar. Se guarda en el documento como `ub:note`; OpenBoard normal no lo muestra.

## Compilación

GitHub Actions compila LibertyBoard para Windows en cada cambio de la rama `master` (`.github/workflows/windows-build.yml`). El resultado se descarga como artefacto `LibertyBoard-windows` desde la pestaña Actions.
