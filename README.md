# Whisk3D UI

**Whisk3D UI** es una biblioteca ligera de interfaces gráficas multiplataforma construida sobre **Whisk3D Core**. Proporciona los componentes básicos necesarios para crear interfaces de usuario para aplicaciones y videojuegos.

El proyecto nació originalmente por la necesidad de contar con una única interfaz gráfica que funcionara de la misma manera en **Windows, Linux, Android y Symbian**, evitando mantener una implementación distinta para cada plataforma. Con el tiempo, muchas partes del código demostraron ser útiles de forma independiente, por lo que Whisk3D UI terminó convirtiéndose en un proyecto separado que puede reutilizarse en cualquier aplicación basada en Whisk3D Core.

**NOTA: este proyecto aun noesta listo para produccion.** actualmente me encuentro limpiando y simplificando este codigo ya que tiene mucho codigo que es de Whisk3D editor. por ejemplo el tema de los colores o que ituliza o esta pensado para usar el atlas de Whisk3D Editor. Seguramente haga ejemplos de uso del codigo y de paso haga una limpieza del codigo.

En lugar de ser un framework de interfaz complejo, Whisk3D UI busca ofrecer las herramientas fundamentales que la mayoría de los proyectos necesitan, entre ellas:

* Botones
* Renderizado de texto
* Ventanas y paneles
* Barras de desplazamiento (scrollbars)
* Ventanas modales
* Notificaciones
* Componentes básicos para construir interfaces

La biblioteca está diseñada para funcionar de forma consistente con distintos métodos de entrada, incluyendo:

* Pantallas táctiles
* Mouse
* Teclado

Su objetivo es ofrecer una solución simple, ligera y portátil, siguiendo la misma filosofía de Whisk3D Core: escribir el código una sola vez y reutilizarlo en múltiples sistemas operativos con la menor cantidad posible de código específico para cada plataforma.

Al proporcionar únicamente los elementos esenciales, Whisk3D UI puede utilizarse tanto en aplicaciones como en videojuegos, sirviendo como base para construir interfaces adaptadas a las necesidades de cada proyecto.

## ¿Es obligatorio usar Whisk3D UI?

No. Whisk3D UI es completamente opcional y podés crear tu propia interfaz de usuario.

La mayoría de las bibliotecas modernas utilizan gráficos vectoriales o tecnologías como OpenVG. Sin embargo, muchos dispositivos antiguos, como los Symbian S60 de primera generación, no aceleran estas tecnologías por hardware y terminan renderizando la interfaz por software, lo que afecta seriamente el rendimiento.

Por este motivo, Whisk3D UI utiliza atlas de texturas bitmap para renderizar texto y controles, permitiendo aprovechar al máximo la aceleración 3D disponible en dispositivos como el Nokia N95 y facilitando futuros ports a hardware retro.

Este enfoque tiene algunas limitaciones frente a las interfaces vectoriales, pero ofrece un rendimiento mucho mejor en plataformas con recursos limitados. Aun así, su uso es opcional y puede reemplazarse por cualquier otra solución de UI/UX.

Whisk3D UI se distribuye bajo la licencia MIT
