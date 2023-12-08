#!/bin/bash

TOP_DIR=$(pwd)
TOOLS_PATH=arm-none-eabi-
FFMPEG_LIB=/home/lmx/sdk/1126/buildroot/output/rockchip_rv1126_rv1109_libs/target/usr/lib/
FFMPEG_INCLUDE=/home/lmx/sdk/1126/buildroot/output/rockchip_rv1126_rv1109_libs/target/usr/include
MPP_LIB=/home/sch/rv1126/linux/buildroot/output/rockchip_rv1126_rv1109_libs/staging/usr/lib/
MPP_INCLUDE=/home/sch/rv1126/linux/buildroot/output/rockchip_rv1126_rv1109_libs/staging/usr/include
RGA_INCLUDE=/home/sch/rv1126/linux/buildroot/output/rockchip_rv1126_rv1109_libs/staging/usr/include
SDK_PATH=/home/lmx/sdk/1126ipc/media/rockit/rockit
THIRD_LIB_PATH=linux_arm32
OS_TYPE=OS_RTT
CPU=ARM32
PROCESSOR=armv7l
SOC=rk3308

function build_rkaduio(){
	echo "build rkaudio"
	cp audio_server.h.in audio_server.h
	cmake -DCMAKE_C_COMPILER_WORKS=ON\
	-DCMAKE_CXX_COMPILER_WORKS=ON\
	-DCMAKE_SYSTEM_NAME=Linux\
	-DCMAKE_SYSTEM_PROCESSOR=${PROCESSOR}\
	-DCMAKE_C_COMPILER=${TOOLS_PATH}gcc\
	-DCMAKE_CXX_COMPILER=${TOOLS_PATH}g++\
	-DCMAKE_INSTALL_LIBDIR=${TOP_DIR}/install/usr\
	-DCMAKE_INSTALL_PREFIX=${TOP_DIR}/install/usr\
	-DCMAKE_BUILD_TYPE=Release\
	-D${OS_TYPE}=ON -DARCH=${CPU}\
	-DRKAUDIO_ENDECODE=ON\
	-DRKAUDIO_DECODER_MP3=ON\
	-DRKAUDIO_ENCODER_MP3=ON\

	make -j32
	if [ $? -eq 0 ]; then
		echo "====Build success!===="
	else
		echo "====Build failed!===="
		exit 1
	fi
}

function build_copy(){
	#Copy Image
	make install
	${TOOLS_PATH}strip --strip-debug ./install/usr/librkaudio_static.a
	pushd ./install
	tar cvf ./rkaudio.tar ./usr
	popd
	echo "====install success!===="
}

build_rkaduio
build_copy
