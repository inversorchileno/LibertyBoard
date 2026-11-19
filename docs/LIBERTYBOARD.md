# LibertyBoard

LibertyBoard es una pizarra digital y lector PDF con puntero láser estilo Drawboard PDF y una interfaz tradicional, en ventana. Es una versión modificada de OpenBoard (GPLv3). Este archivo reúne lo acordado con el dueño del proyecto, para que cualquier sesión de trabajo nueva pueda retomarlo.

## Etapas

1. **Compilación para Windows, láser nuevo y orden de las barras.** *(hecha)*
2. **Paneles de opciones estilo Drawboard y formas nuevas.** *(hecha)*
3. **Rediseño a ventana normal con menús (modo pizarra y modo documentos).** *(hecha)*
4. **Identidad propia: nombre, logo, carpeta de datos y documentación.** *(hecha)*
5. **Menús, guardar, actualización, zoom, notas de colores, idiomas y triángulos de opciones.** *(hecha, compilaciones 9 a 13)*
6. **Pestañas, Guardar/Guardar como, girar, sellos, buscar, notas rápidas, adjuntos y sonidos.** *(hecha, compilación 14)*
7. **Logo nuevo, íconos a color, scroll continuo, pantalla completa, barra que cabe en la pantalla.** *(hecha, compilación 15)*
8. **Mejoras del Word de Luis, en 5 versiones.** *(Versión 1 hecha, compilación 20)*

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
- **Nota:** un clic pone un marcador tipo cinta con un texto. Al pasar el puntero se ve el texto; al hacer clic (con la herramienta Nota, Selección o Interactuar) se puede actualizar, borrar o cambiar de color. Se guarda en el documento como `ub:note` con `ub:color`; OpenBoard normal no lo muestra.

## Compilación y publicación

- GitHub Actions compila LibertyBoard para Windows en cada push a `master` (`.github/workflows/windows-build.yml`), salvo cuando el push solo cambia documentación (`docs/**`, `*.md`). Qt 6.8.3 MSVC, qmake (`OpenBoard.pro` y los `.pri`), librerías de `OpenBoard-ThirdParty`, QuaZip compilado en el mismo flujo.
- Cada compilación de `master` se publica como release `build-<número de ejecución>` con `LibertyBoard-windows.zip`, y ese número queda compilado en el programa (`LIBERTYBOARD_BUILD`). El menú **Acerca → Actualización** (`src/core/UBGitHubUpdater`) compara ese número con el último release, descarga el zip y lo instala con un script de PowerShell que espera a que LibertyBoard se cierre, copia los archivos y lo vuelve a abrir.
- Las traducciones se generan con `lrelease` **antes** de `qmake`; si no, los `.qm` no se instalan y el programa queda en inglés. La compilación falla si falta `OpenBoard_es.qm`.
- Los archivos nuevos se registran en **los dos** sistemas: qmake (`src/*/*.pri`, para Windows) y CMake (`src/*/CMakeLists.txt`, para las pruebas en Linux). Los recursos nuevos van en `resources/OpenBoard.qrc`.
- Una compilación tarda unos 15 a 20 minutos. Se sigue con `gh api repos/inversorchileno/LibertyBoard/actions/runs`.

## Identidad de LibertyBoard

- **Nombre.** El programa se llama LibertyBoard en todo lo visible: ventana, menús, mensajes, ayudas de inicio, traducciones e información del ejecutable de Windows (`LibertyBoard.exe`). El nombre interno de algunos archivos del código (`OpenBoard.pro`, `OpenBoard.qrc`, `OpenBoard_es.ts`) se mantiene para no romper la compilación ni facilitar traer mejoras de OpenBoard.
- **Logo** (desde la compilación 15): cuadrado azul de esquinas redondeadas con una pizarra naranja, firma azul y un lápiz sostenido como antorcha. Es la imagen que entregó Luis, recortada sin el blanco de alrededor (esquinas transparentes): `docs/images/libertyboard-logo.png` (1024 px). Los iconos del programa (`resources/images/OpenBoard.png`, `bigOpenBoard.png`, `ch.openboard.OpenBoard.svg`, `resources/win/LibertyBoard.ico`) se generan desde ella.
- **Carpeta propia.** El nombre de aplicación y de organización es LibertyBoard, así que los documentos y la configuración del usuario quedan en `%LOCALAPPDATA%\LibertyBoard` y el archivo de usuario se llama `LibertyBoardUser.config`. No se lee ni se copia nada de una instalación de OpenBoard: quien instala LibertyBoard parte de cero. La configuración de fábrica está en `resources/etc/LibertyBoard.config` y el estilo en `LibertyBoard.css`.
- **Sin actualizaciones de OpenBoard.** La búsqueda de actualizaciones está apagada y los enlaces de inicio y tutorial apuntan al repositorio de LibertyBoard.
- **Créditos.** La pestaña Acerca de muestra primero LibertyBoard y luego indica que es una versión modificada de OpenBoard, con sus créditos y la licencia GPLv3. Los encabezados de copyright de los archivos de código se mantienen tal cual, como exige la licencia.

## Cómo quedó la Etapa 5

- **Menús** (4): **Inicio** (Nueva pizarra, Abrir/Importar, Guardar, Guardar como, Preferencias, Multipantalla, Podcast, Cerrar documento, Salir; desde la compilación 14), **Modo** (Pizarra, Web, Escritorio), **Edición** (Tamaño de página, Cortar, Copiar, Pegar, Ocultar LibertyBoard, Dormir) y **Acerca** (Tutorial, Consejos, LibertyBoard en GitHub, Actualización). Código en `src/gui/UBMainMenu.cpp`.
- **Guardar y Cerrar documento:** ver la Etapa 6.
- **Zoom:** recuadro fijo abajo a la derecha de la pizarra con −, % editable y + (`src/gui/UBZoomPalette`). Ya no hay zoom en la barra de herramientas.
- **Rueda del mouse en PDF:** al pasar el final de la página va a la siguiente desde arriba, y al pasar el inicio a la anterior desde abajo, con el mismo zoom (`UBBoardView::turnPageWithWheel`). No es una vista continua.
- **Orden de herramientas** (escritorio y pizarra): Lápiz, Borrador, Rotulador, Láser, Resaltar, Interactuar, Nota, Subrayar, Texto, [Línea, solo pizarra], Flecha, Rectángulo, Elipse; en pizarra al final las dos manos (Interactuar con contenidos y Mano); en escritorio al final la **captura de pantalla** (cámara), que guarda pantalla + rayas en `Imágenes\LibertyBoard`.
- **Triángulos de opciones:** solo en modo escritorio; la pizarra no los lleva (sus opciones están en la fila de arriba y con doble clic). Todos negros con borde blanco (`src/gui/UBToolOptionsArrows`), dibujados a la resolución de la pantalla para que los iconos no se pixelen con el escalado de Windows. Apuntan hacia donde se abren las opciones: barra en la mitad izquierda de la pantalla → apunta a la derecha, abajo-derecha del icono; en la mitad derecha → apunta a la izquierda, abajo-izquierda. La ventana de opciones se abre hacia ese mismo lado. (El código también sabe dibujarlos arriba y abajo, por si la barra de escritorio se pone horizontal algún día.)
- **Opciones:** iconos estilo Drawboard en Básico/Pro/Caligrafía, Borde/Relleno, Estela/Punto y Estándar/Brillo blanco; la vista previa del lápiz cambia según el tipo de trazo; el láser muestra los títulos y valores encima de sus barras; la sección de color se llama "Color".
- **Notas de colores:** 7 colores (amarillo, rojo, naranjo, verde, azul, morado, gris), elegibles en la barra de la herramienta Nota y en la ventana de la nota. Las notas van en la capa de los dibujos, sobre el PDF, y salen en el PDF guardado.
- **Idioma:** Preferencias → Mostrar → Idioma (primera opción): idioma del sistema o uno de los 33. Se aplica al reiniciar. Todo texto nuevo de LibertyBoard se traduce a los 33 idiomas en un commit `i18n:` aparte.
- **Preferencias** con fondo claro y letras negras; LibertyBoard fuerza el tema claro (Qt 6.8) porque el modo oscuro de Windows dejaba texto oscuro sobre fondo oscuro. Desde la compilación 14 tienen solo las pestañas General (antes Mostrar, con idioma y presión del lápiz y del rotulador), Cuadrícula, Red, Licencias y Acerca de (con los créditos).
- **Multipantalla** apagada por defecto.
- **Portable:** el dueño usa el zip, sin instalador. Para que aparezca en programas que listan aplicaciones (como Stream Dock), se pone un acceso directo en `shell:common programs`.

## Cómo quedó la Etapa 6

- **Pestañas de documentos** (`src/gui/UBDocumentTabs`): una fila entre el menú y la barra de opciones, visible solo con 2 o más documentos abiertos. Clic cambia de documento (vuelve a la página donde estaba), la X lo cierra (pregunta si guardar cuando hubo cambios) y + abre una pizarra nueva. Un punto • junto al nombre indica cambios sin guardar. Una pizarra en blanco sin usar se reemplaza por el archivo que se abre después.
- **Abrir/Importar** (Ctrl+O) abre la ventana de Windows para elegir archivos (varios a la vez) y los abre en pestañas; ya no lleva al modo Documentos. Un archivo ya abierto se muestra en su pestaña. **Nueva pizarra** (Ctrl+N) está en Inicio.
- **Guardar** (Ctrl+S): sin preguntar, en el archivo de la pestaña: el PDF original con las anotaciones, o para una pizarra la imagen donde se guardó esa página. La primera vez en una pizarra, y con **Guardar como** (Ctrl+Mayús+S), se abre la ventana de Windows (PNG por defecto, JPEG o PDF). Se escribe primero un archivo aparte y solo al terminar reemplaza al original. Aparece «Guardado» 3 segundos a la izquierda de Buscar. El archivo de cada pestaña se recuerda mientras el programa está abierto.
- **Girar** (`src/board/UBPageRotation`): bajo el número de página, dos flechas y la palabra Girar. Gira la página con todo lo que tiene encima, un cuarto de vuelta; vertical ↔ horizontal; se puede deshacer. El PDF guardado sale con la página girada. Las pizarras nuevas siguen partiendo horizontales (decisión de Luis).
- **Pantallazo:** en escritorio, toda la pantalla menos la barra de tareas; en pizarra (botón de cámara al final de la barra), solo la pizarra, sin barras ni paletas (`src/gui/UBScreenshot`). La zona capturada se oscurece un momento, como en Windows.
- **Sello** (`src/domain/UBStamps`): herramienta después de Nota, en escritorio (con triángulo) y en pizarra (fila de sellos en la barra superior). 14 sellos tipo PDF-XChange, en inglés, o en español si LibertyBoard está en español (no pasan por las traducciones). El último, Personalizada, pide un PNG y lo ajusta al tamaño de los demás, con margen. Se ponen como imagen: se mueven, borran y deshacen.
- **Buscar** (Ctrl+F, botón a la izquierda de Escritorio, `src/gui/UBSearchBar` y `src/pdf/UBPdfTextSearch`): busca el texto en el PDF (sin distinguir mayúsculas), marca en amarillo y lleva a cada resultado; «2 de 7» o «No encontrado». En una pizarra sin PDF avisa que solo funciona en PDF con texto. Ctrl+F dejó de ser el atajo de la herramienta Selección.
- **Nota rápida, Adjuntar archivo y Sonido** (solo pizarra, `src/domain/UBGraphicsPageMarkItem` y derivadas): íconos que mantienen su tamaño en pantalla, se mueven, se abren con un clic y se borran con Supr tras clicarlos. Se guardan en la página como `ub:comment`, `ub:attachment` y `ub:sound`; OpenBoard normal no los muestra. En el PDF guardado salen como íconos.
  - Nota rápida: globo amarillo con ventana (fecha y texto); una nota vacía se borra. Con la herramienta, la barra superior ofrece Anterior, Siguiente, Lista de comentarios, Mostrar (ocultar/mostrar todas) y Fusionar comentarios (cada nota pasa a ser una imagen fija con su fecha y texto).
  - Adjuntar archivo: clip; el archivo se copia dentro del documento (`attachments/`) y un clic lo abre con su programa (Excel, etc.).
  - Sonido: altavoz; la ventana permite grabar con el micrófono o examinar un archivo de audio, escucharlo y aceptarlo; se copia al documento (`sounds/`) y un clic lo reproduce o detiene.
- **Colores** en la barra superior: dos filas de muestras más pequeñas y sin la palabra «Color».
- **Zoom:** mostrar u ocultar la barra de opciones ya no vuelve al 100 %; se mantienen el zoom y el lugar.

## Cómo quedó la Etapa 7

- **Logo nuevo** (ver Identidad).
- **Íconos** de Sello, Nota rápida, Adjuntar archivo y Sonido con el estilo de Bolígrafo/Borrador: objetos a color con degradado y sombra (`resources/images/stylusPalette`). El ícono de **Nota** es la cinta que queda en la página, del color elegido para la próxima nota (`src/gui/UBNoteToolIcon`); cambia al cambiar el color.
- **Interactuar** (el dedo con chispa) salió de la barra de la pizarra.
- **Barras que caben:** la barra vertical (pizarra y escritorio) achica sus botones cuando la pantalla no alcanza (`UBActionPalette::fitInLength`); los triángulos se achican con ellos.
- **Pestañas:** la fila mide lo mismo que las pestañas (24 px).
- **Pantalla completa** (Edición, F11): oculta el menú y la barra de tareas; la fila de pestañas se muestra siempre en ese modo, con «Salir de pantalla completa» a la derecha.
- **Sello → Quitar:** botón junto al título; mientras está activo, un clic en un sello lo quita al instante (también en escritorio). Se reconocen los sellos puestos durante la sesión; los de una página reabierta se borran con Selección.
- **Pantallazo en pizarra:** el oscurecimiento es una capa dentro de la ventana (en Windows la ventana aparte quedaba detrás).
- **Grabar sonido:** curva del micrófono en la ventana de Sonido (`src/gui/UBSoundWave`): plana en silencio, onda al hablar.
- **Barra de desplazamiento del documento** (`src/gui/UBDocumentScrollBar`): en el borde derecho de la pizarra, abarca todo el documento hoja tras hoja; se arrastra para ir rápido a cualquier parte, con el mismo zoom. Solo con 2 o más páginas y solo mientras el puntero está cerca del borde derecho; luego se oculta. La barra de herramientas a la derecha le deja espacio.
- **Íconos con el color elegido:** Bolígrafo, Rotulador, Láser y Resaltar cambian sus partes de color al color con que dibujan (`src/gui/UBToolColorIcon`), igual que la Nota.
- **Mano** justo encima de la Nota en la barra de la pizarra.
- **Goma:** dibujos para Completo (trazo punteado que desaparece entero) y Parcial (trazo con un hueco), en pizarra y escritorio.
- **Texto en escritorio:** con Selección, un clic sobre un elemento (texto, su marco o sus botones) lo usa como en la pizarra: cerrar, cambiar tamaño, girar, mover; un clic en lo vacío sigue pasando al escritorio.
- **Scroll continuo** (`UBBoardView::turnPageWithWheel`, `drawNeighbourPages`): la hoja anterior y la siguiente se dibujan pegadas a la actual (16 px de separación); la rueda sigue a su ritmo y, cuando la vecina ocupa el centro, pasa a ser la hoja actual sin mover la imagen. Sigue siendo una hoja a la vez por dentro (no es una vista continua completa).

## Doble pizarra (compilación 16)

- **Modo → Doble pizarra** (`src/board/UBDualBoard`): dos documentos lado a lado (dos PDF, dos pizarras, o un PDF y una pizarra de notas).
- Solo un lado es la pizarra a la vez, el **activo**, con marco azul oscuro: ahí trabajan las herramientas, las páginas, guardar y la barra de desplazamiento. El otro lado (marco claro, `UBOtherBoardView`) muestra su documento como quedó y se puede desplazar; un clic en él, o en su pestaña, lo vuelve activo sin moverlo, con su zoom y lugar.
- La **línea divisoria** se arrastra: parte en 50/50 y cada lado queda entre 20 % y 80 %.
- **Dos documentos como máximo:** con uno abierto, el otro lado es una pizarra nueva (con su pestaña); con más de dos, un aviso pide elegir cuáles cerrar (el actual queda). Abrir un tercero estando en el modo pregunta lo mismo. Cerrar uno de los dos vuelve a pizarra simple; al salir del modo, el otro documento queda en las pestañas.
- Íconos en los menús nuevos (Guardar como, Doble pizarra, Pantalla completa, GitHub); todo menú nuevo lleva su ícono.
- Flecha, Rectángulo y Elipse muestran en su ícono el color de borde y el relleno elegidos (`src/gui/UBShapeToolIcon`).
- Preferencias → Paleta del lápiz: Arriba, Abajo, Izquierda o Derecha (antes Horizontal/Vertical); se puede seguir arrastrando.
- La fila de pestañas tiene la misma holgura arriba y abajo.
- **Inicio → Nueva pizarra** (antes «Nuevo documento»), rotulador con dibujo de destacador tipo Stabilo.

## Lector de PDF completo (compilación 17)

- **Reabrir y recientes** (`src/core/UBOpenedFiles`): abrir de nuevo un PDF ya trabajado (mismo archivo, sin cambios desde que se guardó) retoma su documento de LibertyBoard, con notas, adjuntos y sonidos, sin preguntar si reemplazar. **Inicio → Archivos recientes** muestra los 10 últimos.
- **Inicio → Imprimir** (Ctrl+P, `src/board/UBDocumentPrinter`): diálogo del sistema con rango de páginas o página actual; cada página con su orientación. Ctrl+P ya no elige el bolígrafo.
- **Recuperación** (`UBDocumentTabs`): si el programa se cierra sin terminar, al abrir pregunta si recuperar los documentos que estaban en las pestañas. El autoguardado es cada minuto.
- **Páginas / Borrar página** (`src/gui/UBPageButtons`): dos botones uno sobre otro donde estaba «Páginas», con una línea punteada a su izquierda. «Borrar página» pregunta antes y no funciona con una sola página. El botón de la derecha se llama «Limpiar página»: quita todo lo agregado (el PDF queda) y con pulsación larga da más formas de borrar.
- **Inicio → Extraer páginas** (`src/board/UBPageExtractor`): página actual, todas o rangos («1-3, 5, 8-»); PDF en un archivo, PDF por página, PNG o JPEG por página; nombre, carpeta y abrir al terminar. El PDF sale con el mismo motor que Guardar (texto y giro del original, anotaciones encima). Los exportadores PDF aceptan una lista de páginas (`setPages`).
- **Búsqueda con lista** (`UBSearchBar`): con Buscar abierto, un recuadro a la derecha lista cada resultado con su página y la línea donde aparece; clic para ir. Se actualiza mientras se escribe.
- **Vista continua real** (`UBBoardView::drawNeighbourPages`): se dibujan todas las páginas que caben (hasta 8 a cada lado); con zoom pequeño la rueda avanza página a página con las demás alrededor.
- **Copiar texto del PDF**: con la flecha de selección, un recuadro donde no hay objetos copia el texto del PDF (palabras completas) al portapapeles.
- **Índice y enlaces** (`src/pdf/UBPdfNavigation`, `src/gui/UBPdfOutlinePanel`): botón **Índice** junto a Buscar con el índice del PDF; con la flecha, clic en un enlace del PDF lleva a la página, o pregunta antes de abrir una dirección web. Buscar e Índice se turnan el mismo lugar.
- **Lazo** (`UBDrawingController::isLassoSelection`): la flecha tiene opciones; el botón «Lazo» cambia a selección a mano alzada y el ícono de la paleta pasa a ser un lazo; entonces el botón ofrece «Flecha». En el escritorio la flecha sigue sin opciones.
- **Reconocer formas** (`src/domain/UBShapeRecognizer`): casilla en las opciones del bolígrafo; línea, flecha (dibujada de un trazo, con la punta al final o al principio), círculo, óvalo, triángulo, cuadrado, rectángulo y polígonos de hasta 6 lados se vuelven limpios al levantar el lápiz.
- **Pantalla táctil** (`UBBoardView::viewportEvent`): dos dedos hacen zoom y desplazan; un dedo funciona como el mouse; se ignora la palma (contacto grande) y todo toque mientras se usa el lápiz y un momento después. **No probado en Linux** (sin pantalla táctil).
- **Notas y adjuntos dentro del PDF** (`src/pdf/UBPdfAnnotations`): al guardar o extraer, las notas rápidas quedan como comentarios del PDF y los archivos y sonidos como adjuntos del PDF, en su lugar; otros lectores los muestran. Los íconos de LibertyBoard ya no se dibujan en el PDF (sí al imprimir y en imágenes).
- **Compatibilidad con poppler:** la compilación de Windows usa poppler 23.01 (OpenBoard-ThirdParty) y Linux 24.02; lo que difiere entre versiones se resuelve con sobrecargas (ver `UBPdfNavigation.cpp`, `UBPdfTextSearch.cpp`, `UBPdfAnnotations.cpp`).
- **Español completo:** el archivo de traducción no tiene textos sin traducir (incluidos 180 heredados de OpenBoard) y la ventana de consejos tiene versión en español (`resources/startupHints/locales/es`). Otros idiomas aún tienen textos de OpenBoard sin traducir.
- **Modo → Pizarra** sale de la doble pizarra; ambos documentos quedan en sus pestañas.

## Impresión, OCR y pestañas (compilación 18)

- **Imprimir** (`src/board/UBDocumentPrinter`, clase `UBPrintDialog`): ventana propia con vista previa hoja por hoja. Impresora (las del sistema o «Archivo PDF») y botón «Ajustes de la impresora» (el diálogo del sistema para bandeja, calidad…); copias e intercalar; todas, actual o rangos («1-3, 5, 8-»); papel (Carta, Oficio, A4, A3, A5); orientación automática por página, vertical u horizontal; ajustar a la hoja o tamaño real; 1, 2 o 4 páginas por hoja; color o escala de grises; una o dos caras (si la impresora puede); imprimir o no las anotaciones. Recuerda impresora, papel, escala, color y caras.
- **OCR** (`src/pdf/UBOcrEngine*`, `src/pdf/UBPdfOcr`, `src/gui/UBOcrRunner`): **Inicio → Reconocer texto (OCR)** lee las páginas sin texto del documento (300 ppp), con avance y botón Detener; al abrir un PDF cuyas primeras páginas son solo imágenes, ofrece hacerlo (un «No» se recuerda para ese PDF). En Windows usa el lector de Windows (Windows.Media.Ocr, C++/WinRT, `windowsapp.lib`) en los idiomas del usuario; en Linux, `tesseract` si está instalado (`tesseract-ocr` y `tesseract-ocr-spa`). Las palabras quedan en `objects/<pdf>.ocr.json` junto al PDF del documento; Buscar y Copiar texto (recuadro o lazo) las usan. Al guardar/extraer, el PDF lleva el texto invisible bajo cada página escaneada (`UBPdfAnnotations::writeTextLayer`), también con la página girada.
- **Índice y Buscar** se cierran al volver a apretar su botón (o Ctrl+F).
- **Pestañas:** espacio bajo los nombres y una línea blanca continua de 1 px entre pestañas y barra de opciones.
- **Lazo:** rodear texto sin objetos también lo copia.

## Versión 1 de las mejoras del Word (compilación 20)

Luis entregó un Word con 26 mejoras («mejoras liberty.docx», con capturas de referencia) y se acordó hacerlas en versiones: 1) funciones nuevas y simples, 2) consumo medio, 3) consumo alto, 4) menús Herramientas y Personalización, atajos e IA, 5) vista continua. No se pasa a la siguiente versión hasta que Luis pruebe la anterior y lo pida.

- **Cuenta nueva:** el repositorio pasó de `noteimportaaaa` a `inversorchileno`. El actualizador, los enlaces y la documentación usan la dirección nueva; las versiones antiguas siguen encontrando las nuevas porque GitHub responde con una redirección 301 que el actualizador sigue.
- **Barra de escritorio** (`UBDesktopPalette`): soltada cerca del borde de arriba o de abajo queda horizontal (triángulos de opciones abajo o arriba de los íconos); cerca de la izquierda o la derecha, vertical. Recuerda orientación y borde. Un **triangulito rojo** en el lado que mira al borde de la pantalla la reduce al ícono comprimido con un clic (empujarla contra el borde izquierdo o derecho sigue comprimiéndola). Al final, una **papelera** que borra todo lo dibujado en la pantalla.
- **Borde en modo escritorio** (`src/desktop/UBDesktopFrame`): ventana propia, transparente a los clics, con un borde alrededor de la pantalla. Preferencias → General → «Borde en modo escritorio»: mostrar o no, color y grosor de 1 a 12 px (por defecto rojo, 6 px). Se oculta al volver a la pizarra y al sacar pantallazos.
- **Pestañas:** la franja entre el menú y las pestañas bajó de 18 a 11 px (las pestañas heredaban un margen de 6 px del estilo de OpenBoard). El espacio bajo los nombres y la línea blanca siguen igual.
- **Agregar página:** el botón «Páginas», sobre «Borrar página», se llama «Agregar página».
- **Estilo plantilla** (antes «Fondos», `UBBackgroundPalette`): ventana opaca con 3 plantillas (lisa, cuadriculada, rayada) y debajo una fila «Color de la hoja» (blanco, negro, crema, amarillo, celeste, verde, rosado, azul oscuro, verde oscuro y «…» para cualquier color). Las plantillas negras de OpenBoard ya no se ofrecen: la hoja negra se elige con el color. Cambiar de plantilla mantiene el color. Un color oscuro hace la hoja «oscura» para los trazos y la cuadrícula. Se guarda en la página como `ub:background-color` y como relleno del rectángulo de fondo; OpenBoard la abre blanca o negra.
- **Flecha cónica:** casilla «Cónica» en las opciones de la flecha (barra superior en pizarra, triángulo en escritorio). La línea crece de fina a 2,4 veces su grosor en la punta, con una cabeza mayor, en un solo contorno.
- **Láser arcoíris:** casilla «Arcoíris» aparte de los colores; la estela toma los colores del arcoíris a lo largo de su recorrido, con su brillo; en modo Punto los colores cambian con el tiempo. Elegir un color la apaga.
- **Consejos y trucos:** la primera página muestra el logo de LibertyBoard en vez del globo de OpenBoard.
- **Rotulador:** «Punta circular» o «Punta recta», con dibujo. Con la recta, al levantar el lápiz el trazo se rehace como con la punta biselada de un destacador: ancho en líneas horizontales, más fino en las verticales, con extremos rectos.

## Forma de trabajo

- El dueño es **Luis** (Chile), analista financiero, sin experiencia en programación. Habla en español; se le responde en español, claro y sin jerga. Usa Windows 11, pantalla ancha (unos 2540 px) y lápiz digital.
- Pide cambios en tandas, a menudo con capturas marcadas en verde. Si dice "no compiles aún", se hace commit sin push a `master` (o push a otra rama).
- Antes de cada push: compilar en Linux y probar lo visible en la aplicación real, con capturas. Al terminar, decirle qué se probó y qué no, y qué debe revisar él en Windows.
- Push a `master` = nueva versión para él. Se le avisa del número de compilación y que actualice con **Acerca → Actualización**.
- Commits en inglés con el formato `tipo: resumen` y un cuerpo que explica qué se hizo y en qué se probó (ver `CODING_STYLE.md`). Nunca usar sus documentos reales para pruebas.
- Respetar la GPLv3: se mantienen los encabezados de copyright y la mención "versión modificada de OpenBoard".

## Pruebas en Linux

- Compilar con CMake + Ninja (Qt 6 del sistema) e instalar con `ninja install` (queda como `/usr/local/bin/openboard`).
- Pantalla virtual: `Xvfb :99 -screen 0 2540x1000x24 -dpi 96` + `openbox`; abrir con `HOME=<carpeta de prueba> LANG=es_ES.UTF-8 LANGUAGE=es QTWEBENGINE_DISABLE_SANDBOX=1 LIBGL_ALWAYS_SOFTWARE=1`. Clics con `xdotool`, capturas con `import -window root`.
- Para abrir un PDF de prueba: ejecutar un segundo `openboard <archivo.pdf>`, que lo pasa al programa ya abierto. No reutilizar el mismo nombre de PDF (pregunta si reemplazar).

## Ideas pendientes

- **Instalador** (Inno Setup, hay un script de OpenBoard en `release_scripts/windows`): por ahora el dueño prefiere el portable.
- Importar un PDF con el mismo nombre que uno ya importado, pero cambiado por otro programa, sigue preguntando si reemplazar (si no cambió, se retoma su documento desde la compilación 17).
- Firmas y formularios PDF: descartado por Luis.
- La grabación y la reproducción de sonido no se pudieron probar en Linux (sin dispositivo de audio).
- Se vio una vez que el programa se cerraba al abrir un segundo PDF mientras aparecía esa pregunta de reemplazo (viene de OpenBoard); revisar.
- La ventana de bienvenida (consejos) se abre al inicio hasta que se desmarca.
