/** Represents a time duration. */
export class Duration {

  /** The number of ticks per second used to represent a Duration. */
  public static readonly TICKS_PER_SECOND = 1000;

  /** The number of seconds per minute. */
  public static readonly SECONDS_PER_MINUTE = 60;

  /** The number of minutes per hour. */
  public static readonly MINUTES_PER_HOUR = 60;

  /** Represents an infinite duration. */
  public static readonly POS_INFIN = new Duration(Infinity);

  /** Represents a negatively infinite duration. */
  public static readonly NEG_INFIN = new Duration(-Infinity);

  /** Represents a duration of 0. */
  public static readonly ZERO = new Duration(0);

  /** Represents a duration of one second. */
  public static readonly SECOND = new Duration(Duration.TICKS_PER_SECOND);

  /** Represents a duration of one minute. */
  public static readonly MINUTE = new Duration(Duration.TICKS_PER_SECOND *
    Duration.SECONDS_PER_MINUTE);

  /** Represents a duration of one hour. */
  public static readonly HOUR = new Duration(Duration.TICKS_PER_SECOND *
    Duration.SECONDS_PER_MINUTE * Duration.MINUTES_PER_HOUR);

  /** Constructs a time duration from a JSON object. */
  public static fromJson(value: any): Duration {
    if(value === '+infinity') {
      return Duration.POS_INFIN;
    } else if(value === '-infinity') {
      return Duration.NEG_INFIN;
    }
    if(value.length === 0) {
      return new Duration(0);
    }
    let i = 0;
    const sign = (() => {
      if(value[0] === '-') {
        ++i;
        return -1;
      }
      return 1;
    })();
    let hours = 0;
    while(i < value.length) {
      if(value[i] === ':') {
        ++i;
        break;
      }
      hours = 10 * hours + (value.charCodeAt(i) - 48);
      ++i;
    }
    let minutes = 0;
    while(i < value.length) {
      if(value[i] === ':') {
        ++i;
        break;
      }
      minutes = 10 * minutes + (value.charCodeAt(i) - 48);
      ++i;
    }
    let seconds = 0;
    while(i < value.length) {
      if(value[i] === '.') {
        ++i;
        break;
      }
      seconds = 10 * seconds + (value.charCodeAt(i) - 48);
      ++i;
    }
    let decimalPlaces = 1;
    while(i < value.length) {
      decimalPlaces *= 10;
      seconds = 10 * seconds + (value.charCodeAt(i) - 48);
      ++i;
    }
    const ticks = sign * Duration.TICKS_PER_SECOND *
      (seconds / decimalPlaces + Duration.SECONDS_PER_MINUTE * minutes +
      Duration.SECONDS_PER_MINUTE * Duration.MINUTES_PER_HOUR * hours);
    return new Duration(ticks);
  }

  /** Constructs a time duration, prefer to use composition of existing
   *  constants rather than direct construction.
   * @param ticks - The number of ticks.
   */
  constructor(ticks: number) {
    this._ticks = ticks;
  }

  /** The total number of hours this duration represents. */
  public getTotalHours(): number {
    return this._ticks / (Duration.TICKS_PER_SECOND *
      Duration.SECONDS_PER_MINUTE * Duration.MINUTES_PER_HOUR);
  }

  /** The total number of minutes this duration represents. */
  public getTotalMinutes(): number {
    return this._ticks / (Duration.TICKS_PER_SECOND *
      Duration.SECONDS_PER_MINUTE);
  }

  /** The total number of seconds this duration represents. */
  public getTotalSeconds(): number {
    return this._ticks / Duration.TICKS_PER_SECOND;
  }

  /** The total number of ticks used to represent this duration. */
  public get ticks(): number {
    return this._ticks;
  }

  /** Adds two durations together. */
  public add(other: Duration): Duration {
    return new Duration(this._ticks + other._ticks);
  }

  /** Multiplies this duration by a scalar. */
  public multiply(other: number): Duration {
    return new Duration(other * this._ticks);
  }

  /** Tests if two time durations are equal. */
  public equals(other: Duration): boolean {
    return other && this._ticks === other._ticks;
  }

  /** Splits the duration into an hours/minutes/seconds component. */
  public split() {
    let ticks = Math.abs(this._ticks);
    const sign = Math.sign(ticks);
    let hours = sign * Math.trunc(ticks / (Duration.MINUTES_PER_HOUR *
      Duration.SECONDS_PER_MINUTE * Duration.TICKS_PER_SECOND));
    ticks -= Duration.MINUTES_PER_HOUR * Duration.SECONDS_PER_MINUTE *
      Duration.TICKS_PER_SECOND * hours;
    let minutes = Math.trunc(ticks / (
      Duration.SECONDS_PER_MINUTE * Duration.TICKS_PER_SECOND));
    if(hours === 0) {
      minutes *= sign;
    }
    ticks -= Duration.SECONDS_PER_MINUTE * Duration.TICKS_PER_SECOND * minutes;
    let seconds = ticks / Duration.TICKS_PER_SECOND;
    if(hours === 0 && minutes === 0) {
      seconds *= sign;
    }
    return {
      hours: hours,
      minutes: minutes,
      seconds: seconds
    };
  }

  /** Converts a time duration to JSON. */
  public toJson(): any {
    if(this._ticks === Infinity) {
      return '+infinity';
    } else if(this._ticks === -Infinity) {
      return '-infinity';
    }
    let ticks = Math.abs(this._ticks);
    const hours = Math.trunc(ticks / (Duration.MINUTES_PER_HOUR *
      Duration.SECONDS_PER_MINUTE * Duration.TICKS_PER_SECOND));
    ticks -= Duration.MINUTES_PER_HOUR * Duration.SECONDS_PER_MINUTE *
      Duration.TICKS_PER_SECOND * hours;
    const minutes = Math.trunc(ticks / (
      Duration.SECONDS_PER_MINUTE * Duration.TICKS_PER_SECOND));
    ticks -= Duration.SECONDS_PER_MINUTE * Duration.TICKS_PER_SECOND * minutes;
    const seconds = ticks / Duration.TICKS_PER_SECOND;
    const hourComponent = (() => {
      if(hours === 0) {
        return '00';
      } else if(hours < 10) {
        return '0' + hours.toString();
      } else {
        return hours.toString();
      }
    })();
    const minuteComponent = (() => {
      if(minutes === 0) {
        return '00';
      } else if(minutes < 10) {
        return '0' + minutes.toString();
      } else {
        return minutes.toString();
      }
    })();
    const sign = (() => {
      if(this._ticks < 0) {
        return '-';
      }
      return '';
    })();
    return sign.concat(hourComponent).concat(
      ':').concat(minuteComponent).concat(':').concat(seconds.toString());
  }

  private _ticks: number;
}
