import {Date} from './date';
import {Duration} from './duration';

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
    const hours = 10 * (value.charCodeAt(9) - 48) + value.charCodeAt(10);
    const minutes = 10 * (value.charCodeAt(11) - 48) + value.charCodeAt(12);
    let seconds = 10 * (value.charCodeAt(13) - 48) + value.charCodeAt(14);
    let i = 15;
    if(value.length > i && value[i] === '.') {
      ++i;
      let decimalPlaces = 1;
      while(i < value.length) {
        decimalPlaces *= 10;
        seconds = 10 * seconds + (value.charCodeAt(i) - 48);
      }
      seconds /= decimalPlaces;
    }
    return new DateTime(date,
      new Duration(Duration.TICKS_PER_SECOND *
        (60 * 60 * hours + 60 * minutes + seconds)));
  }

  /** Constructs a date/time. */
  constructor(date: Date = Date.NOT_A_DATE,
      timeOfDay: Duration = Duration.ZERO) {
    this._date = date;
    this._timeOfDay = timeOfDay;
  }

  /** Tests if two date/times represent the same point in time. */
  public equals(other: DateTime): boolean {
    return other && this._date.equals(other._date) && this._timeOfDay.equals(
      other._timeOfDay);
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
    const TICKS_PER_HOUR = Duration.TICKS_PER_SECOND *
      Duration.SECONDS_PER_MINUTE * Duration.MINUTES_PER_HOUR;
    const TICKS_PER_MINUTE = Duration.TICKS_PER_SECOND *
      Duration.SECONDS_PER_MINUTE;
    let ticks = this._timeOfDay.ticks;
    const hours = Math.trunc(ticks / TICKS_PER_HOUR);
    ticks -= hours * TICKS_PER_HOUR;
    const minutes = Math.trunc(ticks / TICKS_PER_MINUTE);
    ticks -= minutes * TICKS_PER_MINUTE;
    const seconds = ticks;
    const hourComponent = (() => {
      if(hours === 0) {
        return '00';
      } else if(hours < 10) {
        return '0' + hours.toString();
      }
      return hours.toString();
    })();
    const minuteComponent = (() => {
      if(minutes === 0) {
        return '00';
      } else if(minutes < 10) {
        return '0' + minutes.toString();
      }
      return minutes.toString();
    })();
    return this._date.toJson().concat('T').concat(hourComponent).concat(
      minuteComponent).concat(seconds.toString());
  }

  private _date: Date;
  private _timeOfDay: Duration;
}
