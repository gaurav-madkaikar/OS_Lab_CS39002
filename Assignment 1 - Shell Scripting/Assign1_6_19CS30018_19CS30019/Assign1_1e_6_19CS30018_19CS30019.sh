#!/bin/bash

verbose=0
while getopts "v" OPTION
do
    case $OPTION in
    v)  verbose=1
        echo -e "Verbose Mode On!\n"
        ;;
    esac
done

function logGET(){
    if [ "$verbose" -eq 1 ]; then
        echo -e "\n+++Sending HTTP GET request to the server..."
        return 0
    fi
    return 1
}

function logRH(){
    if [ "$verbose" -eq 1 ]; then
        echo -e "\n+++Fetching response headers from the server..."
        return 0
    fi
    return 1
}

export REQ_HEADERS="Accept,Connection,Server,Host" 
logGET && curl -s https://example.com/ -o ./example.html || curl -s https://example.com/ -o ./example.html

echo -ne "IP Address: "
logGET && curl -s http://ip.jsontest.com/ | jq -r ".ip" || curl -s http://ip.jsontest.com/ | jq -r ".ip"

echo -e "\nResponse Header:" 
logRH && curl -s -I http://ip.jsontest.com/ || curl -s -I http://ip.jsontest.com/

RESP_HEADERS=`curl -s http://headers.jsontest.com/`

reqHeaders=(${REQ_HEADERS//,/ })
for req in ${reqHeaders[@]}; do
    VAL=`echo "${RESP_HEADERS}" | jq -r ".${req}"`
    if [ "${VAL}" == "null" ]; then
        echo "${req}: Requested header not present!"
    else
        echo "${req}: $VAL"
    fi
done

for file in ./JSONData/*.json; do
    VALIDITY=`curl -s --data-urlencode "json=$(cat $file)" http://validate.jsontest.com/ | jq -r ".validate"`
    FNAME=`echo "${file}" | awk -F"/" '{print $NF}'`
    if [ "${VALIDITY}" == "true" ]; then
        echo "${FNAME}" >> ./valid.txt
    else    
        echo "${FNAME}" >> ./invalid.txt
    fi
done


