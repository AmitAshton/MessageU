# MessageU Project

## הערה לבודק (רועי רחמני)

כפי שסוכם במייל, הפרויקט מוגש באיחור באישור עד לתאריך 9.11.25.

---

## הוראות הרצה: שרת (Python)

* נווט אל תיקיית `src/server`.
* צור קובץ `myport.info` (לדוגמה: `echo 1234 > myport.info`).
* הפעל את השרת: `python main.py`.
* השרת ירוץ ויאזין לחיבורים.

---

## הוראות הרצה: לקוח (C++)

### 1. קומפילציה (Build)

* התקן סביבת Visual Studio 2019 ו `vcpkg`.
* הרץ ב PowerShell: `vcpkg install cryptopp:x64-windows`
* הרץ ב PowerShell: `vcpkg integrate install`
* פתח את `src/client/client.sln` ב Visual Studio.
* הגדר את הפלטפורמה ל `x64`.
* בצע Build לפרויקט (F7).

### 2. הרצה (Testing)

* קובץ ה `client.exe` יווצר בתיקיית `src/client/x64/Debug`.
* צור תיקיות נפרדות עבור כל לקוח (לדוגמה, "Alice" ו "Bob").
* העתק את `client.exe` לכל אחת מהתיקיות.
* צור קובץ `server.info` בכל אחת מתיקיות הלקוח.
* תוכן הקובץ צריך להיות: `127.0.0.1:1234`.
* הפעל את השרת, ולאחר מכן הפעל כל `client.exe` מהתיקייה הנפרדת שלו.
