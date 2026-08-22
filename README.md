# Share URI Bridge

카카오톡 공유하기의 `kakaolink://send/...` data URI를 캡처해 원본 URI 또는 다른 기기로 전달할 GitHub Pages URL을 클립보드에 복사하는 Android 앱입니다.

## 사용법

1. APK를 설치합니다.
2. launcher의 **KakaoLink Capture 설정**을 열어 복사 방식을 선택합니다.
3. 카카오톡 공유하기를 실행합니다.
4. 앱 선택기에서 **KakaoLink Capture**를 선택합니다.
5. 앱은 설정 화면을 띄우지 않고 즉시 복사한 뒤 종료합니다.

설정 Activity와 공유 capture Activity는 서로 다른 task로 격리됩니다. 설정 화면은 사용자가 launcher에서 직접 연 경우에만 표시되며, 다른 앱으로 전환하면 종료됩니다. 공유 capture는 `Theme.NoDisplay`를 사용해 흰색 창이나 window preview를 그리지 않고, history와 최근 앱에도 남지 않습니다.

설정 기본값은 두 항목 모두 꺼짐입니다.

- **Deep-link 사이트 링크로 복사 — OFF:** 원본 `kakaolink://...` data URI를 그대로 복사합니다.
- **Deep-link 사이트 링크로 복사 — ON:** `https://portable-uri-workshop.github.io/#to=...` URL을 기기 안에서 생성해 복사합니다.
- **민감한 클립보드로 표시 — OFF:** Android clipboard preview/history/sync 목적을 유지합니다.
- **민감한 클립보드로 표시 — ON:** `ClipDescription.EXTRA_IS_SENSITIVE` 의미의 flag를 설정합니다. Android가 preview, history, 기기 간 sync를 제한할 수 있습니다.

클립보드는 자동 삭제하지 않습니다.

## 검증과 프라이버시 경계

앱은 다음 조건을 모두 만족한 Intent만 처리합니다.

- action이 `android.intent.action.VIEW`
- scheme이 `kakaolink`
- host가 `send`
- data URI 길이가 32,768자 이하

조건이 맞지 않으면 클립보드를 변경하지 않고 종료합니다. Android 16 이상의 명시적 Intent에도 filter 일치를 요구하도록 `android:intentMatchingFlags="enforceIntentFilter"`를 사용합니다.

앱에는 INTERNET 권한, telemetry, analytics, file persistence, raw URI logcat 출력이 없습니다. **링크 생성**은 기기 안에서만 일어나지만, 생성된 Pages URL을 실제로 열면 브라우저·redirector·대상 앱이 이후 처리를 담당합니다. Redirector source와 배포 commit은 [portable-uri-workshop.github.io](https://github.com/portable-uri-workshop/portable-uri-workshop.github.io)에서 별도로 검증할 수 있습니다.

원본 URI와 Pages URL에는 서비스별 식별값이나 token이 포함될 수 있으므로 공개 게시하지 마세요. 원 앱이 KakaoTalk package/component를 명시적으로 고정한 Intent를 사용하면 이 앱이 chooser에 나타나지 않을 수 있습니다.

## 빌드

다음 버전을 사용합니다.

- Android SDK 36 / Build Tools `36.1.0`
- NDK `27.0.12077973`
- CMake `3.22.1`
- JDK 17 이상
- Gradle `8.14.4` (공식 distribution SHA-256 고정)

Dependency verification은 strict mode입니다.

```bash
./gradlew --dependency-verification=strict assembleRelease
```

서명 설정이 없으면 unsigned release APK를 생성합니다. 설치 가능한 로컬 test APK는 `assembleDebug`로 빌드할 수 있습니다. 배포 APK는 비공개 signing key로 GitHub Actions의 분리된 signing job에서 서명합니다.

Windows에서 프로젝트 전용 로컬 키가 필요하면 `scripts/prepare-local-signing.ps1`을 실행할 수 있습니다. 생성되는 키·암호 설정은 저장소에 포함되지 않습니다.

## Release APK 검증

v2 release pipeline은 다음 runner 경계를 사용합니다.

1. read-only `build-test` job이 source에서 unsigned APK를 빌드합니다.
2. `release-signing` environment의 새 runner가 signing secret을 사용하고 공개된 expected certificate SHA-256을 확인한 뒤 Artifact Attestation을 생성합니다.
3. 별도 `release` runner가 contents write 권한만으로 Release를 만듭니다.

Expected signing certificate SHA-256은 [release/expected-certificate-sha256.txt](release/expected-certificate-sha256.txt)에 고정돼 있습니다.

```bash
gh attestation verify ShareUriBridge-v2.0.2.apk \
  -R portable-uri-workshop/android-share-relay
apksigner verify --verbose --print-certs ShareUriBridge-v2.0.2.apk
```

## 라이선스

[MIT License](LICENSE)
