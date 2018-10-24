/** Represents a specific calendar day. */
export class Date {

  /** Represents a date infinitely in the past. */
  public static readonly NEG_INFIN = new Date(-9999, 1, 1);

  /** Represents a date infinitely in the future. */
  public static readonly POS_INFIN = new Date(9999, 1, 1);

  /** Represents no date. */
  public static readonly NOT_A_DATE = new Date(-1, 1, 1);

  /** Constructs a Date from a JSON object. */
  public static fromJson(value: any): Date {
    if(value === '+infinity') {
      return Date.POS_INFIN;
    } else if(value === '-infinity') {
      return Date.NEG_INFIN;
    } else if(value === 'not-a-date-time') {
      return Date.NOT_A_DATE;
    }
    let i = 0;
    let year = 1000 * (value.charCodeAt(0) - 48) +
      100 * (value.charCodeAt(1) - 48) + 10 * (value.charCodeAt(2) - 48) +
      (value.charCodeAt(3) - 48);
    let month = 10 * (value.charCodeAt(4) - 48) + (value.charCodeAt(5) - 48);
    let day = 10 * (value.charCodeAt(6) - 48) + (value.charCodeAt(7) - 48);
    return new Date(year, month, day);
  }

  /** Constructs a Date.
   * @param year - The date's year.
   * @param month - The date's month.
   * @param day - The date's day.
   */
  constructor(year: number, month: Date.Month, day: number) {
    this._year = year;
    this._month = month;
    this._day = day;
  }

  /** Returns the year. */
  public year(): number {
    return this._year;
  }

  /** Returns the month. */
  public month(): Date.Month {
    return this._month;
  }

  /** Returns the day. */
  public day(): number {
    return this._day;
  }

  /** Tests if two dates are equal. */
  public equals(other: Date): boolean {
    return other && this._year === other._year &&
      this._month === other._month && this._day === other._day;
  }

  /** Converts this Date to JSON. */
  public toJson(): any {
    if(this.equals(Date.NEG_INFIN)) {
      return '-infinity';
    } else if(this.equals(Date.POS_INFIN)) {
      return '+infinity';
    } else if(this.equals(Date.NOT_A_DATE)) {
      return 'not-a-date-time';
    }
    const monthComponent = (() => {
      if(this._month < 10) {
        return '0'.concat(this._month.toString());
      } else {
        return this._month.toString();
      }
    })();
    const dayComponent = (() => {
      if(this._day < 10) {
        return '0'.concat(this._day.toString());
      } else {
        return this._day.toString();
      }
    })();
    return this._year.toString().concat(monthComponent).concat(dayComponent);
  }

  private _year: number;
  private _month: Date.Month;
  private _day: number;
}

export module Date {

  /** Enumerates the months of the year. */
  export enum Month {
    JANUARY = 1,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER
  }
}
