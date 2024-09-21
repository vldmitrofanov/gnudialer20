export function getDispo(num) {
    num = parseInt(num)
    if (num === 1) return 'New';
    if (num === 2) return 'No Answer';
    if (num === -8) return 'Burned (Machine)';
    if (num === -6) return 'Fax (Machine)';
    if (num === -5) return 'Network Congestion (Machine)';
    if (num === -7) return 'Disconnected (Machine)';
    if (num === -4) return 'Busy (Machine)';
    if (num === -3) return 'Voicemail (Machine)';
    if (num === -2) return 'No Answer (Machine)';
    if (num === -1) return 'Undefined (Machine)';
    if (num === 3) return 'Voicemail';
    if (num === 4) return 'Busy';
    if (num === 0) return 'Callback';
    if (num === 5) return 'Fast Busy';
    if (num === 6) return 'Fax';
    if (num === 7) return 'Disconnected';
    if (num === 8) return 'DNC';
    if (num === 9) return 'Invalid';
    if (num === 10) return 'No Sale';
    if (num === 11) return 'No Sale';
    if (num === 12) return 'SALE';
  }