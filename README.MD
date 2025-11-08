# MessageU — תכנות מערכות דפנסיבי

פרויקט זה מממש מערכת **צ'אט מאובטחת מבוססת שרת לקוח**, כחלק מדרישות הקורס  
**"תכנות מערכות דפנסיבי" (20937)**.

השרת ממומש ב־**Python 3**, והלקוח ממומש ב־**C++11**.

---

## הערה לבודק (רועי רחמני)

כפי שסוכם במייל, הפרויקט מוגש **באיחור באישור עד לתאריך 9.11.25**.

---

## דרישות סביבה (Prerequisites)

### צד שרת (Python)
- **Python 3.x**
- אין צורך בספריות חיצוניות  
  (הפרויקט משתמש בספריות מובנות בלבד: `socket`, `sqlite3`, `threading` וכו').

### צד לקוח (C++)
- מערכת הפעלה **Windows**  
- **Visual Studio 2019** (כנדרש ב־PDF של הפרויקט)
- **ספריית Crypto++**: יש להתקין באמצעות `vcpkg`

#### התקנה דרך PowerShell:
```bash
# התקנת Crypto++
.\vcpkg install cryptopp:x64-windows

# קישור אוטומטי ל-Visual Studio
.\vcpkg integrate install
Winsock: הספרייה מקושרת אוטומטית דרך הקוד (NetworkManager.h).

הוראות הרצה
כדי לבדוק את המערכת, יש להריץ את השרת ולאחר מכן שני לקוחות (או יותר) במקביל.

הרצת השרת (Python)
פתח חלון טרמינל (CMD או PowerShell).

נווט לתיקיית השרת:

bash
Copy code
cd src/server
צור קובץ myport.info המכיל את הפורט הרצוי (למשל 1234):

bash
Copy code
echo 1234 > myport.info
הפעל את השרת:

bash
Copy code
python main.py
השרת יריץ אוטומטית:

קובץ defensive.db (בסיס הנתונים)

קובץ server.log (לוגים)

האזנה לחיבורים בפורט שהוגדר.

קומפילציה והרצת הלקוח (C++)
א. קומפילציה (Build)
פתח את הקובץ:

bash
Copy code
src/client/client.sln
באמצעות Visual Studio 2019.

ודא ש־Solution Platform מוגדר ל־x64
(כדי להתאים לספריית Crypto++ שהותקנה).

בצע Build לפרויקט:

בתפריט: Build > Build Solution

או קיצור: F7

תוצר הקומפילציה:
src/client/x64/Debug/client.exe

ב. הרצה (Testing)
 כל לקוח חייב לרוץ מתיקייה נפרדת כדי לשמור קובץ זהות (my.info) ייחודי.

צור תיקייה לבדיקה, למשל:

makefile
Copy code
C:\MessageU_Test
בתוכה צור שתי תיקיות:

makefile
Copy code
C:\MessageU_Test\Alice
C:\MessageU_Test\Bob
העתק את client.exe לתוך שתי התיקיות (Alice ו־Bob).

צור קובץ server.info בכל אחת מהתיקיות, עם התוכן הבא:

makefile
Copy code
127.0.0.1:1234
 בדיקה בפועל:
פתח טרמינל עבור "Alice":

bash
Copy code
cd C:\MessageU_Test\Alice
client.exe
פתח טרמינל נוסף עבור "Bob":

bash
Copy code
cd C:\MessageU_Test\Bob
client.exe
כעת ניתן לבצע את הפעולות בתפריט (למשל):

רישום משתמש: 110

בקשת רשימת משתמשים: 120

שליחת הודעה ועוד...
