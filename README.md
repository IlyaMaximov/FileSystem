# Написание ФС

**Структура проекта (по папкам):**

1) `fsComponents` - содержит основные компоненты ФС

2) `console` - имитирует работу консоли (в ней описаны основные консольные команды)

3) `server` - работает с ФС и отдаёт ответы в `client`

4) `client` - общается с ФС через `server`


**Структура файловой системы:**

- Смещение составляет - 1 КБ (для загрузщика)

- Суперблок -  1 КБ

- Block Group Descriptors - 1 КБ

- Далее идут 16 групп блоков. В каждой:

    1) Битовая карта блоков - 1 КБ
    
    2) Битовая карта inode - 1 КБ
    
    3) Таблица inode ~ 4096 записей (632 КБ)
    
    4) Data blocks - 8192 записей (8192 КБ)  (Размер одного блока - 1 КБ)


Вся файловая система занимайт примерно 150 МБ.