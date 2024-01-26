#定义变量，包含当前文件下的所有源文件
aux_source_directory(. SRC_LIST)
# aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)

#指定生成可执行文件
# add_executable(ChatServer ${SRC_LIST}
#         ../../include/server/db/db.h
#         db/db.cpp
#         usermodel.cpp)
add_executable(ChatServer ${SRC_LIST} ${MODEL_LIST} db/db.cpp)

#指定可执行文件链接时需要依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient pthread)