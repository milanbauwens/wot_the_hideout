const timer = document.getElementsByClassName("timer");
const date = document.getElementsByClassName("date");
const clock = document.getElementsByClassName("clock");

// Set duration of countdown timer in minutes
const countdownDuration = 30;

// Calculate duration to milliseconds
const countdownDurationMs = countdownDuration * 60 * 1000;

// Set the starting time for the countdown
let countdownStart = Date.now();

// Set the countdown interval
let countdownInterval = setInterval(() => {
    // Calculate the time remaining
    let timeRemaining = countdownDurationMs - (Date.now() - countdownStart);

    if (timeRemaining > 0) {
        // Convert time remaining to seconds
        let secondsRemaining = Math.floor(timeRemaining / 1000);

        // Calculate minutes and seconds
        let minutes = Math.floor(secondsRemaining / 60);
        let seconds = secondsRemaining % 60;

        if (minutes < 5) {
            Array.from(timer).forEach(e => {
                e.classList.add("timer--warning");
            })
        }

        // Display minutes and seconds
        Array.from(timer).forEach(e => {
            e.innerHTML = `${leadingZero(minutes)}:${leadingZero(seconds)}`;
        });
    } else {
        Array.from(timer).forEach(e => {
            e.innerHTML = "00:00";
        });
    }
    showtime();
    // If time remaining is less than 0, clear the interval
}, 1000);

const showtime = () => {
    const time = new Date();
    const day = time.getDay();
    const month = time.getMonth();
    const year = time.getFullYear();
    const shortYear = year.toString().substr(-2);
    const hour = time.getHours();
    const minute = time.getMinutes();
    const timeString = `${leadingZero(hour)}:${leadingZero(minute)}`;
    const dateString = `${leadingZero(day)}/${leadingZero(month)}/${shortYear}`;
    Array.from(date).forEach(e => {
        e.innerHTML = dateString;
    });
    Array.from(clock).forEach(e => {
        e.innerHTML = timeString;
    });
}

const leadingZero = (num) => {
    return num < 10 ? `0${num}` : num;
}

showtime();