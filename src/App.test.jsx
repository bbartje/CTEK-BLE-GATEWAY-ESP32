import { render, screen } from '@testing-library/react';
import App from './App';

test('toont de startpagina', () => {
  render(<App />);
  expect(screen.getByRole('heading', { name: /hier komt jouw app/i })).toBeInTheDocument();
});
