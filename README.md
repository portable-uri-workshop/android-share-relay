# Share URI Bridge

카카오톡 공유하기의 딥링크를 캡처해 다른 기기에서도 바로 열 수 있는 링크를 생성하는 프로그램입니다.

## 사용법

1. APK를 설치합니다.
2. 카카오톡 공유하기를 실행합니다.
3. 앱 선택기에서 **KakaoLink Capture**를 선택합니다.
4. 자동 복사된 HTTPS 링크를 다른 기기로 전달하여 엽니다.

모든 처리는 기기 내부에서 수행되며 네트워크로 전송되지 않습니다. 딥링크 자체에 서비스별 식별값이나 토큰이 포함될 수 있으므로 생성된 링크를 공개적으로 게시하지 마세요.

원 앱이 KakaoTalk 패키지나 컴포넌트를 명시적으로 지정한 Intent를 사용하면 앱 선택기에 나타나지 않을 수 있습니다.

## 빌드

Android SDK 35, NDK `27.0.12077973`, CMake `3.22.1`, JDK 17이 필요합니다.

```bash
./gradlew assembleRelease
```

서명 설정이 없으면 Gradle은 서명되지 않은 release APK를 생성합니다. 설치 가능한 로컬 테스트 APK는 `./gradlew assembleDebug`로 빌드할 수 있습니다. 배포 APK는 개인 서명 키를 사용해 별도로 서명해야 하며 키와 `keystore.properties`는 저장소에 포함하지 마세요.

Windows에서 이 프로젝트 전용 로컬 서명 키를 처음 만들 때는 `scripts/prepare-local-signing.ps1`을 한 번 실행한 뒤 다시 `assembleRelease`를 실행할 수 있습니다. 생성되는 키와 설정은 `.gitignore`로 제외됩니다.

## Release APK 검증

공식 Release APK는 버전 태그의 소스를 GitHub Actions가 직접 빌드하고 서명합니다. APK의 SHA-256은 함께 첨부된 `.sha256` 파일로 확인할 수 있습니다.

GitHub Artifact Attestation으로 APK가 이 저장소의 워크플로와 커밋에서 생성됐는지 검증할 수 있습니다.

```bash
gh attestation verify ShareUriBridge-v1.0.0.apk \
  -R portable-uri-workshop/android-share-relay
```

## 라이선스

[MIT License](LICENSE)
