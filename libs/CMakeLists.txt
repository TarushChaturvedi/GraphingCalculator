include(FetchContent)

FetchContent_Declare(
    sfml
    GIT_REPOSITORY "https://github.com/SFML/SFML"
    GIT_TAG 7f1162dfea4969bc17417563ac55d93b72e84c1e # 3.0.0
)

add_subdirectory(sfml)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui"
    GIT_TAG dbb5eeaadffb6a3ba6a60de1290312e5802dba5a # v1.91.8 
)

FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY "https://github.com/SFML/imgui-sfml"
    GIT_TAG 5e8f8fb857c8f3cff1096bf3e4402c64e3bcf6af # v3.0
)

add_subdirectory(imgui-sfml)
