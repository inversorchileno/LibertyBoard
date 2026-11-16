# LibertyBoard

LibertyBoard es un fork de OpenBoard con un puntero láser estilo Drawboard PDF y una interfaz más tradicional, en ventana. Este archivo reúne lo acordado con el dueño del proyecto, para que cualquier sesión de trabajo nueva pueda retomarlo.

## Etapas

1. **Compilación para Windows, láser nuevo y orden de las barras.** *(en curso)*
2. Paneles de opciones estilo Drawboard y formas nuevas.
3. Rediseño a ventana normal con menús (modo pizarra y modo documentos).

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

## Compilación

GitHub Actions compila LibertyBoard para Windows en cada cambio de la rama `master` (`.github/workflows/windows-build.yml`). El resultado se descarga como artefacto `LibertyBoard-windows` desde la pestaña Actions.
