# 📫 VEDA-TCP/IP Chat
> TCP/IP를 이용한 채팅 프로그램.

<br>

![VEDA_TALK](https://github.com/user-attachments/assets/6b863e9e-6f41-40ed-83e1-f495132638dd)

</br>

#### ✉ 여러명의 사람들과 즐거운 대화를 나누세요!
- 회원 가입을 하고 계정을 생성합니다.
- 등록한 ID와 PASSWORD로 로그인을 하고 대화방에 참가합니다.
- LOGOUT 입력을 통해 로그아웃이 가능합니다.

<br>

## 🛠 개발 환경
| 개발 툴 | 개발 언어 | 개발 환경 | 컴파일러 |
| :------: |  :------: | :------: | :------: |
|![vieditor](https://github.com/user-attachments/assets/271f00d2-604d-4dc8-8a85-0681a278498a)|![C](https://github.com/user-attachments/assets/3e16350e-a963-462c-b2dc-9dfff9b93eb6)|![linux](https://github.com/user-attachments/assets/39c54fd4-d93a-4379-9ef6-d27067c06acd)|![make](https://github.com/user-attachments/assets/8a0a3321-35d1-4a2c-bdcb-c9a98f3158f7)|

<br>

## 🧾 시스템 순서도
![flow](https://github.com/user-attachments/assets/12512a0c-f887-48b2-a44b-2eb6e1cf8954)

<br>

## 🧬 시스템 아키텍처
![archi](https://github.com/user-attachments/assets/117724c4-0602-4448-b1e6-5ff97d7b47e7)

<br>


## 🎥 기능 구현
| 메인 화면 | 회원 가입 | 로그인 |
| :------: |  :------: | :------: |
|![VEDA_TALK](https://github.com/user-attachments/assets/bb6f3ec2-e67a-4105-ba0e-89a446920488)|![회원가입](https://github.com/user-attachments/assets/f39826cd-406d-4b10-8d7f-41a702932879)|![로그인 성공](https://github.com/user-attachments/assets/bd7f38e8-ffa4-4806-a134-ed60bff06d3d)



<br>

## ✍ 회고 (4L)

- 좋았던 점 (Liked)

    - 이번 프로젝트를 통해 TCP/IP와 통신 프로토콜에 대해 깊이 있게 생각할 수 있었습니다
    - 소켓 통신과 비동기 I/O를 사용해 서버를 구현하면서 네트워크 프로그래밍의 기본을 배울 수 있었습니다. 
    - LINUX 환경에서 작업하면서 시스템 프로그래밍에 대한 이해도가 높아졌고, 터미널 명령어 사용에도 익숙해졌습니다.

<br>

- 힘들었던 점 (Lacked)

    - fork()로 생성된 자식 프로세스가 처리 과정에서 죽지 않을 때가 종종 있어 좀비 프로세스가 되지 않도록 처리하는 것이 생각보다 까다로웠습니다.
    - 소켓, 파이프, fork를 함께 사용하다 보니 각기 다른 방식으로 데이터를 송수신하는 과정이 복잡하고 까다로웠습니다. 특히, 동기와 비동기 통신을 혼합해 사용하는 부분에서 예상치 못한 문제들이 발생하여 어려움을 겪었습니다.

<br>

- 배운 점 (Learend)

    - 이번 프로젝트를 통해 TCP/IP의 기본 개념을 되짚어볼 수 있었고 서버프로세스에 대해서도 이해할 수 있었습니다.
    - 서버와 클라이언트 간 통신을 구현하면서, 프로세스 관리와 시스템 자원 사용에 대해 더 깊이 이해하게 되었습니다.
    - C 언어 코딩에 아직 부족한 부분이 많다는 것을 깨닫고, 효율적인 코드 작성과 디버깅 능력을 키워야겠다는 생각을 하게 되었습니다.

<br>

- 향후 개선점 (Longed For)

    - 앞으로는 서버 성능을 더 잘 평가하기 위해 단위 테스트와 부하 테스트를 진행할 계획입니다 이를 통해 서버의 성능을 최적화하고, 안정성을 높일 수 있을 것입니다.
    - 초기 명령어 처리 부분을 switch문으로 간소화하여 코드의 가독성과 유지보수성을 향상시킬 예정입니다.
