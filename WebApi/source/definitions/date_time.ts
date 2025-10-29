import { Date } from './date';
import { Duration } from './duration';

/** Represents a point in time. */
export class DateTime {

  /** Represents a point in time infinitely in the future. */
  public static readonly POS_INFIN = new DateTime(Date.POS_INFIN);

  /** Represents a point in time infinitely in the past. */
  public static readonly NEG_INFIN = new DateTime(Date.NEG_INFIN);

  /** Represents no date/time. */
  public static readonly NOT_A_DATE_TIME = new DateTime(Date.NOT_A_DATE);

  /** Constructs a date/time from a JSON object. */
  public static fromJson(value: any): DateTime {
    if(value === '+infinity') {
      return DateTime.POS_INFIN;
    } else if(value === '-infinity') {
      return DateTime.NEG_INFIN;
    } else if(value === 'not-a-date-time') {
      return DateTime.NOT_A_DATE_TIME;
    }
    const date = Date.fromJson(value);
    const hours = 10 * (value.charCodeAt(9) - 48) + value.charCodeAt(10) - 48;
    const minutes =
      10 * (value.charCodeAt(11) - 48) + value.charCodeAt(12) - 48;
    let seconds = 10 * (value.charCodeAt(13) - 48) + value.charCodeAt(14) - 48;
    let i = 15;
    if(value.length > i && value[i] === '.') {
      ++i;
      let decimalPlaces = 1;
      while(i < value.length) {
        decimalPlaces *= 10;
        seconds = 10 * seconds + (value.charCodeAt(i) - 48);
        ++i;
      }
      seconds /= decimalPlaces;
    }
    return new DateTime(date, new Duration(
      Duration.TICKS_PER_SECOND * (60 * 60 * hours + 60 * minutes + seconds)));
  }

  /** Constructs a date/time. */
  constructor(
      date: Date = Date.NOT_A_DATE, timeOfDay: Duration = Duration.ZERO) {
    this._date = date;
    this._timeOfDay = timeOfDay;
  }

  /** Returns the date. */
  public get date(): Date {
    return this._date;
  }

  /** Returns the time of day. */
  public get timeOfDay(): Duration {
    return this._timeOfDay;
  }

  /** Tests if two date/times represent the same point in time. */
  public equals(other: DateTime): boolean {
    return other && this._date.equals(other._date) &&
      this._timeOfDay.equals(other._timeOfDay);
  }

  /** Converts this DateTime to a JavaScript Date. */
  public toDate(): globalThis.Date {
    if(this._date.equals(Date.POS_INFIN)) {
      return new globalThis.Date(8640000000000000);
    } else if(this._date.equals(Date.NEG_INFIN)) {
      return new globalThis.Date(-8640000000000000);
    } else if(this._date.equals(Date.NOT_A_DATE)) {
      return new globalThis.Date(NaN);
    }
    const {hours, minutes, seconds} = split(this._timeOfDay.ticks);
    return new globalThis.Date(`${this._date.year}-` +
      `${padZeros(this._date.month)}-${padZeros(this._date.day)}T` +
      `${padZeros(hours)}:${padZeros(minutes)}:${padZeros(seconds)}Z`);
  }

  /** Converts this date/time to JSON. */
  public toJson(): any {
    if(this._date.equals(Date.POS_INFIN)) {
      return '+infinity';
    } else if(this._date.equals(Date.NEG_INFIN)) {
      return '-infinity';
    } else if(this._date.equals(Date.NOT_A_DATE)) {
      return 'not-a-date-time';
    }
    const {hours, minutes, seconds} = split(this._timeOfDay.ticks);
    return this._date.toJson().concat('T').concat(padZeros(hours)).
      concat(padZeros(minutes)).concat(padZeros(seconds));
  }

  public toString(): string {
    return this.toDate().toLocaleString();
  }

  private _date: Date;
  private _timeOfDay: Duration;
}

function padZeros(value: number) {
  if(value === 0) {
    return '00';
  } else if(value < 10) {
    return '0' + value.toString();
  }
  return value.toString();
}

function split(ticks: number) {
  const TICKS_PER_HOUR = Duration.TICKS_PER_SECOND *
    Duration.SECONDS_PER_MINUTE * Duration.MINUTES_PER_HOUR;
  const TICKS_PER_MINUTE = Duration.TICKS_PER_SECOND *
    Duration.SECONDS_PER_MINUTE;
  const hours = Math.trunc(ticks / TICKS_PER_HOUR);
  ticks -= hours * TICKS_PER_HOUR;
  const minutes = Math.trunc(ticks / TICKS_PER_MINUTE);
  ticks -= minutes * TICKS_PER_MINUTE;
  const seconds = ticks / Duration.TICKS_PER_SECOND;
  return {hours, minutes, seconds};
}
