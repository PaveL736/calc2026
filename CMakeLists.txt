cmake_minimum_required(VERSION 3.16)
project(calc2026 VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#________________________________________________________________________
# находим компоненты Qt
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Quick
    Widgets
    Qml
)

#________________________________________________________________________
# брабатываем ТОЛЬКО ресурсы (иконки, шрифты, SVG)
# в resources.qrc должны быть только бинарные или текстовые ассеты.
# сюда нельзя добавлять .qml файлы!
qt6_add_resources(QML_RESOURCES_RCC resources.qrc)

#________________________________________________________________________
# создаем исполняемый файл
# в список исходников добавляем:
# - main.cpp (точка входа)
# - calculator.cpp/.h (логика калькулятора)
# - сгенерированный файл ресурсов (${QML_RESOURCES_RCC})
add_executable(appcalc2026
    main.cpp
    calculator.cpp
    calculator.h
    ${QML_RESOURCES_RCC}
)

#________________________________________________________________________
# включаем автоматическую обработку для исполняемого файла
set_target_properties(appcalc2026 PROPERTIES
    AUTOMOC ON   # Для Q_OBJECT в calculator.h
    AUTOUIC ON   # Для ui-файлов (если будут)
    AUTORCC ON   # Для ресурсов (добавили .cpp вручную, но это не повредит)
    WIN32_EXECUTABLE TRUE
)

#________________________________________________________________________
# линкуем библиотеки
target_link_libraries(appcalc2026 PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    Qt6::Widgets
)
