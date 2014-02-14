## Herramientas automáticas

Para cargar la ruta en el _path_ y poder ejecutar las herramientas sin tener que buscar dónde están, ejecutamos `source setpath` en la terminal.

### Tests

La herramienta `catest` genera tests automáticamente. Hay tres posibles modos:

* `catest init` inicializa un directorio de tests en la carpeta actual.
* `catest suite <nombre>` crea una suite de tests en la carpeta `tests` de la carpeta actual. Los archivos creados serán `test_<nombre>.h` y `test_<nombre>.c`, y se añadirá al fichero `test.c` las instrucciones necesarias para ejecutar la nueva suite.
* `catest test <suite> <test1> <test2> ...` crea uno o varios tests (el número de argumentos es variable) dentro de la suite dada.

El binario generado con `make test` permite excluir o incluir suites de tests. Sin argumentos, ejecutará todos los tests encontrados. Con `test exclude suite1 suite2 ...` ejecutará todas las suites salvo las especificadas, y con `test include suite1 suite2 ...` ejecutará sólo las suites especificadas.

### Creando un test

Una vez que tenemos las funciones de tests generadas, simplemente las rellenamos. En `testmacros.h` hay varias macros para hacer las aserciones (`mu_assert, mu_assert_eq`). Además, hay algunas utilidades para cuando tenemos que liberar memoria antes de salir del test (`mu_cleanup_fail` y `mu_cleanup_sysfail`), para marcar fallos directamente (`mu_fail` y `mu_sysfail`). Los tests deben de acabar con `mu_end` para que lo reconozca todo bien.
