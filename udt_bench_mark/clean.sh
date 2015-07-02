OLD_PWD="$( pwd )"

    cd ./udt_dgram_server/ && make clean && \
    cd ../udt_dgram_client/ && make clean && \
    echo ""

# restore old path.
cd $OLD_PWD
