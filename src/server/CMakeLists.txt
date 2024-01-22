#定义变量，包含当前文件下的所有源文件
aux_source_directory(. SRC_LIST)

#指定生成可执行文件
add_executable(ChatServer ${SRC_LIST}
        ../../include/server/db/db.h
        db/db.cpp
        usermodel.cpp)

#指定可执行文件链接时需要依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base /usr/lib64/mysql/libmysqlclient.so pthread)