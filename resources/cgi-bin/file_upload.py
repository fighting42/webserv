#!/usr/bin/env python3
import os
import sys
import cgi
from datetime import datetime

# 환경 변수에서 업로드 디렉토리 경로를 가져옴
upload_dir = os.environ.get('UPLOAD_PATH', 'resources/upload')

# 입력한 경로가 존재하지 않는다면 생성
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir)

form = cgi.FieldStorage()

# 현재 날짜와 시간을 포맷팅하여 파일 이름에 추가
current_datetime = datetime.now().strftime("%Y%m%d%H%M%S")

# 요청 본문을 읽어서 파일 이름과 데이터를 추출하여 저장
for key in form.keys():
    if form[key].filename:
        filename = current_datetime + '_' + form[key].filename
        filedata = form[key].file.read()

        # 업로드된 파일을 저장할 경로
        file_path = os.path.join(upload_dir, filename)
        with open(file_path, 'wb') as f:
            f.write(filedata)

        print("파일 '{}' 업로드 완료.".format(filename))
        print("파일 경로: {}".format(file_path))
    else:
        print("파일을 선택해주세요.")
