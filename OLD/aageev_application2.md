# Агеев Артем - "3D Drawer"

### Группа: 10 - МИ - 5
### Электронная почта: artem.artem64@yandex.ru
### VK: https://vk.com/artem_78


**[ НАЗВАНИЕ ПРОЕКТА / ОБЛАСТЬ]**

“3D Vision”
IT –проект

**[ ПРОБЛЕМНОЕ ПОЛЕ ]**

Для проведения многих экспериментов и тестов, ученым необходимо знать точную траекторию различных объектов. Физики занимающиеся проблемами в области аэродинамики, при запуске макетов могут довольствоваться лишь приблизительными оценками, так как программного обеспечения, решающего задачи получения траекторий объектов нету.

**[ ЗАКАЗЧИК / ПОТЕНЦИАЛЬНАЯ АУДИТОРИЯ ]**
 
 * Физики в области аэродинамики и космических исследований
 * Биологи занимающиеся изучением траекторий полёта птиц (орнитологи)
 * Инженеры в областях проектирования БЛА (беспилотные летательные аппараты) и авиации
 * Военные разработки
 
**[ АППАРАТНЫЕ ТРЕБОВАНИЯ ]** 

OS - Linux
CPU - Intel i5-8250U и лучше
GPU - Intel HD graphics 620

**[ ФУНКЦИОНАЛЬНЫЕ ТРЕБОВАНИЯ ]**

Для работы системы необходимы две камеры достаточно большого разрешения и частоты съёмки. Камеры необходимо установить параллельно друг другу (линии центров обзора камер параллельны) на произвольном расстоянии большем 10см так, чтобы область движения камер попала в поле зрения обеих камер. Для повышения точности необходимо использовать широкоугольные камеры и расставлять и на большем расстоянии.

 **[ РАБОТА С КЛИЕНТАМИ / АУДИТОРИЕЙ ]**
 
Данный проект появился из идеи проекта '3D Drawer'. При показе автором работы проекта 3D Drawer на различных выставках, многие замечали потенциал использования его не в сфере рисования, где уже существует огромное количество программных продуктов, но в сфере мониторинга траекторий произвольных объектов. Многие физики, услышав про идею проекта поддержали его развитие в этом направлении.

**[ ПОХОЖИЕ / АНАЛОГИЧНЫЕ ПРОДУКТЫ ]**

В качестве альтернативы, аналогичных продуктов нет. Для осуществления этих задач использовалось востановление 3D координат вручную с результатов съемки стереокамер или использование 'маячков' с аппаратурой, позволяющей определить положение самого маячка.

**[ ИНСТРУМЕНТЫ РАЗРАБОТКИ ]**

* Codeblocks + mingw + OpenGL + OpenCV

**[ ЭТАПЫ РАЗРАБОТКИ ]**

0) Разработка плана переноса проекта '3D Drawer' на эту идею
1) Июнь - завершение работ созданию фреймворка по обнаружению произвольных объектов
2) Июль - внедрение в проект системы облачного хранения данных для работы с несколькими установками / перевод системы на вычисления на видеокарте
3) Август - внедрение дополнительного пользовательского интерфейса

**[ ВОЗМОЖНЫЕ РИСКИ ]**

А. Сложности и нецелесообразность внедрения в проект системы облачного хранения данных
Б. Проблемы с поворотом объектов при использовании гистограм ориентированных градиентов
