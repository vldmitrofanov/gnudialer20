/* Calendar.java:  Java program to display a calendar for any month
 *   after 1582. The user inputs the year into a TextField and chooses
 *   the month from a Choice object (drop-down combo box), then clicks
 *   'New Calendar' to show the calendar for that month and year. Call
 *   applet from an HTML file with this tag:
 *
 *    <applet code>="Calendar.class" width=421 height=481></applet>
 *
 *   The applet assumes a 421x481 pixel surface on which to draw.
 */
import java.applet.Applet;
import java.awt.*;
import java.util.Date;

public class Calendar extends Applet
  {
  static final int YTOP = 90;      /* y-size of margin above calendar box */
  static final int YHEADER = 30;   /* y-size of horz strip with day names */
  static final int NCELLX = 7;     /* number of cells across */
  static final int CELLSIZE = 60;  /* size of each square cell */
  static final int MARGIN = 8;     /* margin from number to cell top, right */
  static final int FEBRUARY = 1;   /* special month during leap years */

  // Data entry controls at top.
  Label yearLabel = new Label("Year:");
  TextField yearTextField = new TextField("1996", 5);
  Label monthLabel = new Label("Month:");
  Choice monthChoice = new Choice();
  Button newCalButton = new Button("New Calendar");

  // Date object to get current month and year.
  Date now = new Date();

  // Font for controls at top.
  Font smallArialFont = new Font("Arial", Font.PLAIN, 15);
  // Font for for month/year caption above calendar.
  Font largeArialFont = new Font("Arial", Font.BOLD, 30);

  String days[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                   "Thursday", "Friday", "Saturday"};

  String months[] = {"January", "February", "March", "April",
                     "May", "June", "July", "August", "September",
                     "October", "November", "December"};

  int DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  // Month and year entered by user.
  int userMonth;
  int userYear;

  public void init()
  /*
     USE:  Get current month and year, initialize controls.
     NOTE: Called automatically when applet starts.
  */
    {
    setBackground(Color.white);

    // Init month and year to current values.
    userMonth = now.getMonth();
    userYear  = now.getYear() + 1900;

    // "Year:" label.
    yearLabel.setFont(smallArialFont);
    add(yearLabel);

    // Text field to get year from user.
    yearTextField.setFont(smallArialFont);
    yearTextField.setText(String.valueOf(userYear));
    add(yearTextField);

    // "Month:" label.
    monthLabel.setFont(smallArialFont);
    add(monthLabel);

    // Combo box to get month from user: add months, set default to now.
    monthChoice.setFont(smallArialFont);
    for (int i = 0; i < 12; i++)
      monthChoice.addItem(months[i]);
    monthChoice.select(userMonth);
    add(monthChoice);

    // "New calendar" button.
    newCalButton.setFont(smallArialFont);
    add(newCalButton);
    } // init

  public void paint(Graphics g)
  /*
     USE:  Draw calendar using globals userMonth and userYear.
     NOTE: Called automatically whenever surface needs to be redrawn;
           also when user clicks 'New Calendar' button, triggering repaint.
  */
    {
    FontMetrics fm;     /* to get font info */
    int fontAscent;     /* character height */
    int dayPos;         /* y-position of day strings */
    int xSize, ySize;   /* size of calendar body (cell table) */
    int numRows;        /* number of rows in cell table (4, 5, 6) */
    int xNum, yNum;     /* number position at top right of cells */
    int numDays;        /* number of days in month */
    String dayStr;      /* day of the week as a string */
    int marg;           /* margin of month string baseline from cell table */
    String caption;     /* month string at top center */

    // Get font info for number string positioning (default small font).
    fm = g.getFontMetrics();
    fontAscent = fm.getAscent();
    dayPos = YTOP + (YHEADER + fontAscent) / 2;

    // Get x-size of calendar body (cell table).
    xSize = NCELLX * CELLSIZE;

    // Header rectangle across top for day names.
    g.drawRect(0, YTOP, xSize, YHEADER);

    // Put days at top of each column, centered.
    for (int i = 0; i < NCELLX; i++)
      g.drawString(days[i], (CELLSIZE-fm.stringWidth(days[i]))/2 + i*CELLSIZE,
                   dayPos);

    // Get number of calendar rows needed for this month.
    numRows = NumberRowsNeeded(userYear, userMonth);

    // Vertical lines of cell table.
    ySize =  numRows * CELLSIZE;
    for (int i = 0; i <= xSize; i += CELLSIZE)
      g.drawLine(i, YTOP + YHEADER, i, YTOP + YHEADER + ySize);

    // Horizontal lines of cell table.
    for (int i = 0, j = YTOP + YHEADER; i <= numRows; i++, j += CELLSIZE)
      g.drawLine(0, j, xSize, j);

    // Init number positions (upper right of cell).
    xNum = (CalcFirstOfMonth(userYear, userMonth) + 1) * CELLSIZE - MARGIN;
    yNum = YTOP + YHEADER + MARGIN + fontAscent;

    // Get number of days in month, adding one if February of leap year.
    numDays = DaysInMonth[userMonth] +
              ((IsLeapYear(userYear) && (userMonth == FEBRUARY)) ? 1 : 0);

    // Show numbers at top right of each cell, right justified.
    for (int day = 1; day <= numDays; day++)
      {
      dayStr = String.valueOf(day);
      g.drawString(dayStr, xNum - fm.stringWidth(dayStr), yNum);
      xNum += CELLSIZE;
      // If xNum to right of calendar, 'new line'.
      if (xNum > xSize)
        {
        xNum = CELLSIZE - MARGIN;
        yNum += CELLSIZE;
        }  // if
      }  // for

    // Set large font for month/year caption.
    g.setFont(largeArialFont);
    // Get font info for string positioning (large font now current).
    fm = g.getFontMetrics();
    // Set margin for y-positioning of caption.
    marg = 2 * fm.getDescent();

    // Set caption to month string and center at top.
    caption = months[userMonth] + " " + String.valueOf(userYear);
    g.drawString(caption, (xSize-fm.stringWidth(caption))/2, YTOP - marg);
    } // paint

  public boolean action(Event e, Object o)
  /*
     USE:  Update month and year globals, paint when user clicks button.
  */
    {
    int userYearInt;

    if (e.target instanceof Button)
      {
      if ("New Calendar".equals((String)o))
        {
        // Get month from combo box (Choice control).
        userMonth = monthChoice.getSelectedIndex();

        // Get year from TextField, update userYear only if year ok.
        userYearInt = Integer.parseInt(yearTextField.getText(), 10);
        if (userYearInt > 1581)
          userYear = userYearInt;

        // Call paint() to draw new calendar.
        repaint();
        return true;
        } // inner if
      } // outer if

    return false;
    } // action

  int NumberRowsNeeded(int year, int month)
  /*
     USE:  Calculates number of rows needed for calendar.
     IN:   year = given year after 1582 (start of the Gregorian calendar).
           month = 0 for January, 1 for February, etc.
     OUT:  Number of rows: 5 or 6, except for a 28 day February with
           the first of the month on Sunday, requiring only four rows.
  */
    {
    int firstDay;       /* day of week for first day of month */
    int numCells;       /* number of cells needed by the month */
  
    /* Start at 1582, when modern calendar starts. */
    if (year < 1582) return (-1);
  
    /* Catch month out of range. */
    if ((month < 0) || (month > 11)) return (-1);
  
    /* Get first day of month. */
    firstDay = CalcFirstOfMonth(year, month);
  
    /* Non leap year February with 1st on Sunday: 4 rows. */
    if ((month == FEBRUARY) && (firstDay == 0) && !IsLeapYear(year))
      return (4);
  
    /* Number of cells needed = blanks on 1st row + days in month. */
    numCells = firstDay + DaysInMonth[month];
  
    /* One more cell needed for the Feb 29th in leap year. */
    if ((month == FEBRUARY) && (IsLeapYear(year))) numCells++;
  
    /* 35 cells or less is 5 rows; more is 6. */
    return ((numCells <= 35) ? 5 : 6);
    } // NumberRowsNeeded
  
  int CalcFirstOfMonth(int year, int month)
  /*
     USE:  Calculates day of the week the first day of the month falls on.
     IN:   year = given year after 1582 (start of the Gregorian calendar).
           month = 0 for January, 1 for February, etc.
     OUT:  First day of month: 0 = Sunday, 1 = Monday, etc.
  */
    {
    int firstDay;       /* day of week for Jan 1, then first day of month */
    int i;              /* to traverse months before given month */
  
    /* Start at 1582, when modern calendar starts. */
    if (year < 1582) return (-1);
  
    /* Catch month out of range. */
    if ((month < 0) || (month > 11)) return (-1);
  
    /* Get day of week for Jan 1 of given year. */
    firstDay = CalcJanuaryFirst(year);
  
    /* Increase firstDay by days in year before given month to get first day
     * of month.
     */
    for (i = 0; i < month; i++)
      firstDay += DaysInMonth[i];
  
    /* Increase by one if month after February and leap year. */
    if ((month > FEBRUARY) && IsLeapYear(year)) firstDay++;
  
    /* Convert to day of the week and return. */
    return (firstDay % 7);
    } // CalcFirstOfMonth
  
  boolean IsLeapYear(int year)
  /*
     USE:  Determines if given year is a leap year.
     IN:   year = given year after 1582 (start of the Gregorian calendar).
     OUT:  TRUE if given year is leap year, FALSE if not.
     NOTE: Formulas capture definition of leap years; cf CalcLeapYears().
  */
    {
  
    /* If multiple of 100, leap year iff multiple of 400. */
    if ((year % 100) == 0) return((year % 400) == 0);
  
    /* Otherwise leap year iff multiple of 4. */
    return ((year % 4) == 0);
    } // IsLeapYear
  
  int CalcJanuaryFirst(int year)
  /*
     USE:  Calculate day of the week on which January 1 falls for given year.
     IN:   year = given year after 1582 (start of the Gregorian calendar).
     OUT:  Day of week for January 1: 0 = Sunday, 1 = Monday, etc.
     NOTE: Formula starts with a 5, since January 1, 1582 was a Friday; then
           advances the day of the week by one for every year, adding the
           number of leap years intervening, because those years Jan 1
           advanced by two days. Calculate mod 7 to get the day of the week.
  */
    {
    /* Start at 1582, when modern calendar starts. */
    if (year < 1582) return (-1);
  
    /* Start Fri 01-01-1582; advance a day for each year, 2 for leap yrs. */
    return ((5 + (year - 1582) + CalcLeapYears(year)) % 7);
    } // CalcJanuaryFirst
  
  int CalcLeapYears(int year)
  /*
     USE:  Calculate number of leap years since 1582.
     IN:   year = given year after 1582 (start of the Gregorian calendar).
     OUT:  number of leap years since the given year, -1 if year < 1582
     NOTE: Count doesn't include the given year if it is a leap year.
           In the Gregorian calendar, used since 1582, every fourth year
           is a leap year, except for years that are a multiple of a
           hundred, but not a multiple of 400, which are no longer leap
           years. Years that are a multiple of 400 are still leap years:
           1700, 1800, 1990 were not leap years, but 2000 will be.
  */
    {
    int leapYears;      /* number of leap years to return */
    int hundreds;       /* number of years multiple of a hundred */
    int fourHundreds;   /* number of years multiple of four hundred */
  
    /* Start at 1582, when modern calendar starts. */
    if (year < 1582) return (-1);
  
    /* Calculate number of years in interval that are a multiple of 4. */
    leapYears = (year - 1581) / 4;
  
    /* Calculate number of years in interval that are a multiple of 100;
     * subtract, since they are not leap years.
     */
    hundreds = (year - 1501) / 100;
    leapYears -= hundreds;
  
    /* Calculate number of years in interval that are a multiple of 400;
     * add back in, since they are still leap years.
     */
    fourHundreds = (year - 1201) / 400;
    leapYears += fourHundreds;
  
    return (leapYears);
    } // CalcLeapYears
  
} // class Calendar

